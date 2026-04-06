#ifndef CANBACKEND_HPP
#define CANBACKEND_HPP

#include <QObject>
#include <QVariantList>
#include <QString>
#include "CANFrame.hpp"
#include "BitStuffing.hpp"
#include "CRC15.hpp"

// CANBackend — bridges C++ CAN library with QML frontend
// Exposed to QML as a singleton via Q_OBJECT and properties

class CANBackend : public QObject {
    Q_OBJECT

    // Properties visible in QML
    Q_PROPERTY(int frameCount   READ frameCount   NOTIFY frameCountChanged)
    Q_PROPERTY(int errorCount   READ errorCount   NOTIFY errorCountChanged)
    Q_PROPERTY(int stuffBitCount READ stuffBitCount NOTIFY stuffBitCountChanged)

public:
    explicit CANBackend(QObject* parent = nullptr);

    // Called from QML when user clicks Build Frame
    Q_INVOKABLE QVariantMap buildFrame(const QString& id,
                                       int dlc,
                                       const QString& data,
                                       const QString& type);

    // Getters
    int frameCount()    const { return m_frameCount; }
    int errorCount()    const { return m_errorCount; }
    int stuffBitCount() const { return m_stuffBitCount; }

    // Reset all counters
    Q_INVOKABLE void reset();

signals:
    void frameCountChanged();
    void errorCountChanged();
    void stuffBitCountChanged();

private:
    int m_frameCount    = 0;
    int m_errorCount    = 0;
    int m_stuffBitCount = 0;

    // Encode frame into bit stream with field colors
    QVariantList encodeBitStream(const CANFrame& frame, int& stuffBits);
};

#endif // CANBACKEND_HPP