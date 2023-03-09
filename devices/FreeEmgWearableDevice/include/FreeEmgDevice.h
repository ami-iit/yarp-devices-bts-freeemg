/**
 * @file FreeEmgDevice.h
 * @authors  Riccardo Grieco <riccardo.grieco@hotmail.com>
 */

#ifndef FREE_EMG_DEVICE_H
#define FREE_EMG_DEVICE_H

#include <Wearable/IWear/IWear.h>

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IPreciselyTimed.h>
#include <yarp/dev/ISerialDevice.h>
#include <yarp/os/PeriodicThread.h>

namespace wearable {
    namespace devices {
        class FreeEmgDevice;
    } // namespace devices
} // namespace wearable

class wearable::devices::FreeEmgDevice
    : public yarp::dev::DeviceDriver
    , public yarp::os::PeriodicThread
    , public yarp::dev::IPreciselyTimed
    , public wearable::IWear
{
private:
    class FreeEmgDeviceImpl;
    std::unique_ptr<FreeEmgDeviceImpl> m_pImpl;

public:
    FreeEmgDevice();
    ~FreeEmgDevice() override;

    // DeviceDriver
    bool open(yarp::os::Searchable& config) override;
    bool close() override;

    // PeriodicThread
    void run() override;
    void threadRelease() override;

    // ================
    // IPRECISELY TIMED
    // ================

    yarp::os::Stamp getLastInputStamp() override;

    // =====
    // IWEAR
    // =====

    // GENERIC
    // -------

    WearableName getWearableName() const override;

    WearStatus getStatus() const override;

    TimeStamp getTimeStamp() const override;

    SensorPtr<const sensor::ISensor> getSensor(const sensor::SensorName name) const override;

    VectorOfSensorPtr<const sensor::ISensor> getSensors(const sensor::SensorType) const override;

    // IMPLEMENTED SENSORS
    // -------------------

    SensorPtr<const sensor::IEmgSensor>
    getEmgSensor(const sensor::SensorName name) const override;

    // UNIMPLEMENTED SENSORS
    // ---------------------

    inline SensorPtr<const sensor::IVirtualLinkKinSensor>
    getVirtualLinkKinSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IVirtualJointKinSensor>
    getVirtualJointKinSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IForce3DSensor>
    getForce3DSensor(const sensor::SensorName name) const override
    {
        return nullptr;    
    }

    inline SensorPtr<const sensor::ITorque3DSensor>
    getTorque3DSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IForceTorque6DSensor>
    getForceTorque6DSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IFreeBodyAccelerationSensor>
    getFreeBodyAccelerationSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IMagnetometer>
    getMagnetometer(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IOrientationSensor>
    getOrientationSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IPoseSensor>
    getPoseSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IPositionSensor>
    getPositionSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IVirtualSphericalJointKinSensor>
    getVirtualSphericalJointKinSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IAccelerometer>
    getAccelerometer(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::IGyroscope>
    getGyroscope(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::ISkinSensor>
    getSkinSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    inline SensorPtr<const sensor::ITemperatureSensor>
    getTemperatureSensor(const sensor::SensorName name) const override
    {
        return nullptr;
    }

    // UNIMPLEMENTED ACTUATORS
    // -----------------------

    ElementPtr<const actuator::IActuator>
    getActuator(const actuator::ActuatorName name) const override
    {
        return nullptr;
    }

    VectorOfElementPtr<const actuator::IActuator>
    getActuators(const actuator::ActuatorType type) const override;

    inline ElementPtr<const actuator::IHaptic>
    getHapticActuator(const actuator::ActuatorName name) const override
    {
        return nullptr;
    }

    inline ElementPtr<const actuator::IHeater>
    getHeaterActuator(const actuator::ActuatorName name) const override
    {
        return nullptr;
    }

    inline ElementPtr<const actuator::IMotor>
    getMotorActuator(const actuator::ActuatorName name) const override
    {
        return nullptr;
    }
};

#endif // HAPTIC_GLOVE_H
