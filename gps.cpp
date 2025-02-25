#include "gps.h"
#include <QDebug>
#include <QThread>

GPS::GPS(QObject *parent)
    : QObject(parent),
    m_serial(new QSerialPort(this)),
    m_latitude(0.0),
    m_longitude(0.0)
{
    // Adjust this port name depending on your OS & device:
    // e.g. "/dev/ttyUSB0" on Linux, "COM3" on Windows
    m_serial->setPortName("COM3");
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    connect(m_serial, &QSerialPort::readyRead, this, &GPS::readData);
}

GPS::~GPS() {
    stop();
}

void GPS::start() {
    if (!m_serial->isOpen()) {
        if (!m_serial->open(QIODevice::ReadWrite)) {
            emit errorOccurred("Failed to open USB port: " + m_serial->errorString());
            return;
        }
        qDebug() << "Serial port opened on" << m_serial->portName();

        // Disable echo (ATE0) and enable GPS (AT+CGPS=1)
        m_serial->write("ATE0\r\n");
        QThread::sleep(1);
        m_serial->write("AT+CGPS=1\r\n");   // Enable GPS
        QThread::sleep(1);

        // Optionally request immediate position
        // m_serial->write("AT+CGPSINFO\r\n");
    }
}

void GPS::stop() {
    if (m_serial->isOpen()) {
        // Disable GPS if desired
        m_serial->write("AT+CGPS=0\r\n");
        QThread::sleep(1);

        m_serial->close();
        qDebug() << "Serial port closed.";
    }
}

// Manually request the module's position with AT+CGPSINFO
void GPS::requestPosition() {
    if (m_serial->isOpen()) {
        m_serial->write("AT+CGPSINFO\r\n");
    } else {
        emit errorOccurred("Serial port is not open.");
    }
}

// Convert "3723.2475" + "N" to decimal degrees
double GPS::parseCoordinate(const QString &coordinate, const QString &direction) {
    if (coordinate.isEmpty()) return 0.0;

    // For example: "3723.2475" => 37 degrees, 23.2475 minutes
    // If direction is S or W, we negate the result
    int dotIndex = coordinate.indexOf('.');
    if (dotIndex < 2) return 0.0;  // Ensure at least 2 digits for degrees

    int degDigits = dotIndex - 2;  // e.g., "37" is degrees
    QString degStr = coordinate.left(degDigits);
    QString minStr = coordinate.mid(degDigits);
    bool ok = false;
    double deg = degStr.toDouble(&ok);
    if (!ok) return 0.0;
    double minutes = minStr.toDouble(&ok);
    if (!ok) return 0.0;

    double decimal = deg + (minutes / 60.0);
    if (direction == "S" || direction == "W") {
        decimal = -decimal;
    }
    return decimal;
}

void GPS::readData() {
    // Read all incoming serial data
    QByteArray data = m_serial->readAll();
    QString response = QString::fromUtf8(data).trimmed();

    // Debug output
    qDebug() << "Raw Data:" << response;

    // If the module returns an error
    if (response.contains("ERROR")) {
        emit errorOccurred("GPS Error: " + response);
        return;
    }

    // Look for +CGPSINFO: lat,N,lon,E,date,time,alt,...
    if (response.contains("+CGPSINFO:")) {
        QStringList parts = response.split(":");
        if (parts.size() < 2) return;
        QString info = parts.at(1).trimmed(); // Extract the GPS data after "+CGPSINFO:"
        QStringList fields = info.split(',');

        if (fields.size() < 7) {  // Ensure at least 7 fields exist
            emit errorOccurred("Incomplete GPS info: " + info);
            return;
        }

        QString latStr = fields.at(0);
        QString latDir = fields.at(1);
        QString lonStr = fields.at(2);
        QString lonDir = fields.at(3);
        QString altStr = fields.at(6);  // Corrected altitude field (7th field)

        // If latStr or lonStr is empty, the GPS fix is not acquired
        if (latStr.isEmpty() || lonStr.isEmpty()) {
            emit errorOccurred("No GPS fix yet.");
            return;
        }

        double newLat = parseCoordinate(latStr, latDir);
        double newLon = parseCoordinate(lonStr, lonDir);
        double newAlt = altStr.toDouble();  // Parse altitude correctly

        if (newLat != m_latitude || newLon != m_longitude || newAlt != m_altitude) {
            m_latitude = newLat;
            m_longitude = newLon;
            m_altitude = newAlt;  // Store altitude
            qDebug() << "GPS Update: Lat:" << m_latitude
                     << " Lon:" << m_longitude
                     << " Alt:" << m_altitude;
            emit positionUpdated();
        }
    }
}

