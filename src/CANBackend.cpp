#include "CANBackend.hpp"
#include <QStringList>

CANBackend::CANBackend(QObject* parent) : QObject(parent) {}

QVariantMap CANBackend::buildFrame(const QString& id, int dlc,
                                   const QString& data, const QString& type) {
    QVariantMap result;

    // Parse ID
    bool ok;
    uint32_t idNum = id.toUInt(&ok, 16);
    if (!ok) {
        m_errorCount++;
        emit errorCountChanged();
        result["valid"] = false;
        result["error"] = "Invalid ID";
        return result;
    }

    // Parse data bytes
    QStringList dataTokens = data.trimmed().split(" ", Qt::SkipEmptyParts);
    uint8_t dataBytes[8] = {};
    int dataLen = qMin(dlc, dataTokens.size());

    for (int i = 0; i < dataLen; i++) {
        dataBytes[i] = dataTokens[i].toUInt(&ok, 16);
        if (!ok) {
            m_errorCount++;
            emit errorCountChanged();
            result["valid"] = false;
            result["error"] = "Invalid data byte: " + dataTokens[i];
            return result;
        }
    }

    // Build CAN frame
    CANFrame frame;
    if (type == "EXT")
        frame = CANFrame::buildExtended(idNum, dataBytes, dlc);
    else if (type == "RTR")
        frame = CANFrame::buildRemote(idNum, dlc);
    else
        frame = CANFrame::buildStandard(idNum, dataBytes, dlc);

    if (!frame.isValid()) {
        m_errorCount++;
        emit errorCountChanged();
        result["valid"] = false;
        result["error"] = "Frame validation failed";
        return result;
    }

    // Encode bit stream
    int stuffBits = 0;
    QVariantList bitStream = encodeBitStream(frame, stuffBits);

    m_frameCount++;
    m_stuffBitCount += stuffBits;
    emit frameCountChanged();
    emit stuffBitCountChanged();

    // Build breakdown
    QVariantList breakdown;
    auto addField = [&](const QString& name, const QString& value, const QString& color) {
        QVariantMap field;
        field["fieldName"]  = name;
        field["fieldValue"] = value;
        field["fieldColor"] = color;
        breakdown.append(field);
    };

    addField("SOF",      "0 (dominant)",                          "#FF4444");
    addField("ID",       "0x" + id.toUpper() + " (" + QString::number(dlc) + "B)", "#00D4FF");
    if (type == "EXT")
        addField("SRR+IDE+RTR", "1 1 0", "#FFB74D");
    else if (type == "RTR")
        addField("RTR+IDE", "1 0", "#FFB74D");
    else
        addField("RTR+IDE", "0 0", "#FFB74D");

    addField("DLC",      QString::number(dlc) + " bytes",         "#81C784");
    addField("DATA", type == "RTR" ? "no data (remote request)" : data.toUpper(), "#CE93D8");
    // Encode frame locally just to get CRC value for display
    uint8_t tempBits[256] = {};
    uint32_t tempLen = frame.encode(tempBits, 256);
    uint16_t crcVal = CRC15::calculate(tempBits, tempLen);
    addField("CRC-15", QString("0x%1").arg(crcVal, 4, 16, QChar('0')).toUpper(), "#F06292");
    addField("STUFF",    QString::number(stuffBits) + " bits inserted", "#FFD54F");

    result["valid"]     = true;
    result["bitStream"] = bitStream;
    result["breakdown"] = breakdown;
    result["stuffBits"] = stuffBits;
    result["frameId"]   = id.toUpper();
    result["dlc"]       = dlc;
    result["data"] = type == "RTR" ? "no data" : data.toUpper();
    result["frameType"] = type;

    return result;
}

QVariantList CANBackend::encodeBitStream(const CANFrame& frame, int& stuffBits) {
    QVariantList result;
    stuffBits = 0;

    // Encode raw bits
    uint8_t rawBits[256] = {};
    uint32_t rawLen = frame.encode(rawBits, 256);

    // Field colors matching QML
    // We'll color by position: SOF=1, ID=11, RTR+IDE+r0=3, DLC=4, rest=data
    struct FieldRange { uint32_t start; uint32_t end; QString color; };

    QVector<FieldRange> ranges;
    uint32_t pos = 0;

    if (frame.getIDType() == CANIDType::EXTENDED) {
        ranges.append({pos, pos,      "#FF4444"}); pos += 1;  // SOF
        ranges.append({pos, pos+10,   "#00D4FF"}); pos += 11; // Base ID
        ranges.append({pos, pos+1,    "#FFB74D"}); pos += 2;  // SRR+IDE
        ranges.append({pos, pos+17,   "#00D4FF"}); pos += 18; // Extended ID
        ranges.append({pos, pos+2,    "#FFB74D"}); pos += 3;  // RTR+r1+r0
        ranges.append({pos, pos+3,    "#81C784"}); pos += 4;  // DLC
        ranges.append({pos, rawLen-1, "#CE93D8"});             // Data
    } else {
        ranges.append({pos, pos,      "#FF4444"}); pos += 1;  // SOF
        ranges.append({pos, pos+10,   "#00D4FF"}); pos += 11; // ID
        ranges.append({pos, pos+2,    "#FFB74D"}); pos += 3;  // RTR+IDE+r0
        ranges.append({pos, pos+3,    "#81C784"}); pos += 4;  // DLC
        ranges.append({pos, rawLen-1, "#CE93D8"});             // Data
    }

    auto getColor = [&](uint32_t idx) -> QString {
        for (const auto& r : ranges)
            if (idx >= r.start && idx <= r.end)
                return r.color;
        return "#E8EEF7";
    };

    // Apply bit stuffing
    uint8_t consecutive = 1;
    uint8_t lastBit = rawBits[0];

    QVariantMap firstBit;
    firstBit["bit"]        = QString::number(rawBits[0]);
    firstBit["fieldColor"] = getColor(0);
    result.append(firstBit);

    for (uint32_t i = 1; i < rawLen; i++) {
        uint8_t bit = rawBits[i] & 1;

        if (bit == lastBit) {
            consecutive++;
        } else {
            consecutive = 1;
        }

        QVariantMap entry;
        entry["bit"]        = QString::number(bit);
        entry["fieldColor"] = getColor(i);
        result.append(entry);

        lastBit = bit;

        if (consecutive == 5) {
            uint8_t stuffBit = !bit;
            QVariantMap stuff;
            stuff["bit"]        = QString::number(stuffBit);
            stuff["fieldColor"] = "#FFD54F";
            result.append(stuff);
            stuffBits++;
            consecutive = 1;
            lastBit = stuffBit;
        }
    }
    // Append CRC-15 bits in pink
    uint16_t crc = CRC15::calculate(rawBits, rawLen);
    for (int i = 14; i >= 0; i--) {
        uint8_t crcBit = (crc >> i) & 1;
        QVariantMap crcEntry;
        crcEntry["bit"]        = QString::number(crcBit);
        crcEntry["fieldColor"] = "#F06292";
        result.append(crcEntry);
    }

    // CRC delimiter (recessive, 1)
    QVariantMap delim;
    delim["bit"]        = "1";
    delim["fieldColor"] = "#F06292";
    result.append(delim);
    return result;
}

void CANBackend::reset() {
    m_frameCount    = 0;
    m_errorCount    = 0;
    m_stuffBitCount = 0;
    emit frameCountChanged();
    emit errorCountChanged();
    emit stuffBitCountChanged();
}