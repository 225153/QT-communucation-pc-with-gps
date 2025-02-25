#ifndef GPS_H
#define GPS_H

#include <QObject>
#include <QSerialPort>
#include <QString>

class GPS : public QObject
{
    Q_OBJECT
    // Expose latitude and longitude to QML
    Q_PROPERTY(double latitude READ latitude NOTIFY positionUpdated)
    Q_PROPERTY(double longitude READ longitude NOTIFY positionUpdated)
    Q_PROPERTY(double altitude READ altitude NOTIFY positionUpdated)

public:
    explicit GPS(QObject *parent = nullptr);
    ~GPS();

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double altitude() const { return m_altitude; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void requestPosition(); // Send AT+CGPSINFO to get latest data

signals:
    void positionUpdated();             // Emitted when new lat/lon is parsed
    void errorOccurred(const QString &message);

private slots:
    void readData();

private:
    // Convert strings like "3723.2475,N" to decimal degrees
    double parseCoordinate(const QString &coordinate, const QString &direction);

    QSerialPort *m_serial;
    double m_latitude;
    double m_longitude;
    double m_altitude;
};

#endif // GPS_H
