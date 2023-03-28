/**
 * @file FreeEmgDevice.cpp
 * @authors  Riccardo Grieco <riccardo.grieco@hotmail.com>
 */

#include <vector>

#include "FreeEmgDevice.h"

#include <yarp/os/LogStream.h>
#include <yarp/os/Property.h>

// Include standard libraries
#include <SDKDDKVer.h>
#include <atlbase.h>
#include <stdio.h>
#include <tchar.h>
#include <process.h>
#include <atomic>

#include "BioDAQUtils.h"

#import "mscorlib.tlb"
#import "bts.biodaq.drivers.tlb"
#import "bts.biodaq.core.tlb"

#include <map>

// Use namespaces
using namespace mscorlib;
using namespace bts_biodaq_core;
using namespace bts_biodaq_drivers;

using namespace wearable;
using namespace wearable::devices;

static const std::string LOG_PREFIX = std::string("FreeEmgDevice");

class SensorParams
{
public:
    SensorParams(unsigned int label,OLE_COLOR color, std::string& macAddress):
        label(label), color(color), macAddress(macAddress)
    {
        labelString = std::to_string(label);
        channelName = "EMG device " + labelString;
    }

    unsigned char label;
    std::string labelString;
    std::string channelName;
    OLE_COLOR color;
    std::string macAddress;
};

// ===========================================
// FreeEmgDevice implementation of EmgSensor
// ===========================================

class FreeEmgSensor
    : public sensor::IEmgSensor
{
public:
    FreeEmgSensor(const sensor::SensorName& name = {},
            const sensor::SensorStatus& status = sensor::SensorStatus::Ok)
        : IEmgSensor(name, status)
    {}

    ~FreeEmgSensor() override = default;

    bool getEmgSignal(double& emgSignal) const override
    {
        emgSignal = this->emgSignal;
        return true;
    }

    void setEmgSignal(double emgSignal)
    {
        this->emgSignal = emgSignal;
    }

    bool getNormalizationValue(double& normalizationValue) const override
    {
        //TODO
        normalizationValue = 1.0;
        return true;
    }

    inline void setStatus(const sensor::SensorStatus& status) { m_status = status; }

private:
    std::atomic<double> emgSignal = 0.0;
};

// ===========================================

class FreeEmgDevice::FreeEmgDeviceImpl
{
public:
    double period = 0.01; //default 100Hz

    std::vector<int> comPorts;

    WearableName wearableName = "FreeEmg";

    std::atomic<double> timeStamp;

    // SDK pointer/objects
    IBioDAQPtr ptrBioDAQ = nullptr;
    IQueueSinkPtr ptrQueueSink = nullptr;

    // Configuration params
    bool useCustomProfile = false;
    std::vector<SensorParams> sensorParams;
    std::vector<unsigned char> labelList; // list of EMG labels to use
    std::vector<std::string> sensorNames; // names associated to labels

    // EMG sensors
    std::string emgSensorPrefix;
    std::map<unsigned char,SensorPtr<FreeEmgSensor>> emgSensorMap;

    bool run();

    // Read parameters functions
    bool readParams(yarp::os::Searchable& config);
    bool readProfileParam(yarp::os::Searchable& config);
    bool readSensorParam(yarp::os::Searchable& config);
    bool readLabelToNames(yarp::os::Searchable& config);

    bool configure(yarp::os::Searchable& config);
    bool configureProfile();
    bool configureProtocol();
    bool configureDataRead();

    bool close();
};

bool FreeEmgDevice::FreeEmgDeviceImpl::configureDataRead()
{
    long nSinks = ptrBioDAQ->Sinks->Count;
    // Do some check: number of sinks
    if (nSinks <= 0)
    {
        //TODO add the sink?
        yError() << LOG_PREFIX << "There are no sinks!";
        return false;
    }

    // Get the specialized IQueueSink interface from generic IDataSink interface
    IDataSink * pDataSinkTmp = ptrBioDAQ->Sinks->GetItem(0);
    pDataSinkTmp->QueryInterface(__uuidof(IQueueSink), (void**) &ptrQueueSink);
    
    // Set properties for all channels
    ptrBioDAQ->ChannelsView->SetEMGChannelsRangeCode(EMGChannelRangeCodes_Gain3_0mV);   // EMG channel range
    ptrBioDAQ->ChannelsView->SetEMGChannelsSamplingRate(SamplingRate_Rate1KHz);         // Sampling rate: 1 KHz
    ptrBioDAQ->ChannelsView->SetEMGChannelsCodingType(CodingType_Raw);                 
    ptrBioDAQ->ChannelsView->SetEMGChannelsCompression(true);                           // Coding type: ADPCM

    // Arm the system
    BioDAQExitStatus exitStatusArm = ptrBioDAQ->Arm();
    if(exitStatusArm!=BioDAQExitStatus_Success){
        yError() << LOG_PREFIX <<"Unable to arm the system!";
        return false;
    }
    yInfo()<<LOG_PREFIX<<"System armed!";

    // Wait 100 ms (from example)
    Sleep(100);

    // Start capturing
    BioDAQExitStatus exitStatusCapturing = ptrBioDAQ->Start();
    if(exitStatusCapturing!=BioDAQExitStatus_Success){
        yError() << LOG_PREFIX <<"Unable to start capturing!";
        return false;
    }
    yInfo()<<LOG_PREFIX<<"System is capturing data!";

    // Wait 250 ms (from example)
    Sleep(250);

    return true;
}

bool FreeEmgDevice::FreeEmgDeviceImpl::configureProtocol()
{
    // Create protocol
    IProtocolPtr ptrProtocol( __uuidof(Protocol) );

    ISensorViewDictionaryPtr ptrSensorViewDictionary = ptrBioDAQ->SensorsView;

    for(int i = 0; i<labelList.size(); i++)
    {
        // Find the sensor in the profile
        ISensorViewDictionaryEnumPtr ptrSensorViewDictionaryEnum = ptrSensorViewDictionary->GetEnumerator();
        bool enumeratorRes = true;
        ISensorViewPtr sensor = nullptr;
        
        while(enumeratorRes && sensor == nullptr){
            enumeratorRes = ptrSensorViewDictionaryEnum->MoveNext() == VARIANT_TRUE ? true : false; 
            if(enumeratorRes && ptrSensorViewDictionaryEnum->Current->LabelCode==labelList[i]){
                sensor = ptrSensorViewDictionaryEnum->Current;
            }            
        }

        if(sensor==nullptr){
            yError()<<LOG_PREFIX<<"Missing sensor with label"<<labelList[i]<<"in sensor dictionary (profile)!";
            return false;
        }

        // Create EMG protocol item
        std::string labelString = std::to_string(labelList[i]);
        IProtocolItemPtr ptrProtocolItem( __uuidof(ProtocolItem) );
        ptrProtocolItem->ChannelName = A2BSTR(sensorNames[i].c_str());                   // channel name
        ptrProtocolItem->SensorColor = sensor->Color;                             // sensor color
        ptrProtocolItem->SensorLabel = A2BSTR(labelString.c_str());                   // sensor label
        ptrProtocolItem->_ChannelType = (ChannelType) ChannelType_EMG;                              // channel type
        ptrProtocolItem->SensorChannelIndex = FreeEmg::BIODAQ_EMG_SENSOR_CHANNEL_IDX; // sensor channel index

        // Add item to protocol
        IProtocolItem * pProtocolItem; 
        ptrProtocolItem.QueryInterface(__uuidof(IProtocolItem), &pProtocolItem);
        
        HRESULT hRes = ptrProtocol->Add(pProtocolItem);
        if(hRes != S_OK)
        {
            yError()<<"Unable to add EMG channel"<<ptrProtocolItem->ChannelName<<"to the protocol!";
            return false;
        }
    }

    // Apply protocol
    BioDAQExitStatus exitStatus = ptrBioDAQ->ApplyProtocol(ptrProtocol);
    if( exitStatus != BioDAQExitStatus_Success )
    {
        yError()<<"Unable to apply new protocol!";
        return false;
    }
    yInfo() << "Protocol applied successfully!";

    return true;
}

bool FreeEmgDevice::FreeEmgDeviceImpl::configureProfile()
{
    // Use the protocol already set
    if(!useCustomProfile){
        return true;
    }

    // Create sensor id dictionary
    ISensorIdDictionaryPtr ptrSensorIdDictionary( __uuidof(SensorIdDictionary) );

    SAFEARRAY* MACBytes = nullptr;
    for(const auto& sensorParam : sensorParams){
        // create sensor and set properties
        ISensorIdPtr ptrSensorId( __uuidof(SensorId) );
        
        MACBytes = FreeEmg::createMacAddressSafeArray(sensorParam.macAddress);

        ptrSensorId->MACBytes = MACBytes;
        ptrSensorId->Color = sensorParam.color; 
        ptrSensorId->LabelCode = sensorParam.label;
        ptrSensorId->Device = SensorType_EMG;

        // Add to dictionary
        ISensorId * pSensorId; 
        ptrSensorId.QueryInterface(__uuidof(ISensorId), &pSensorId);
        HRESULT hRes = ptrSensorIdDictionary->Add(MACBytes, pSensorId);
        if ( hRes != S_OK)
        {
            yError() << "Unable to add EMG sensor [MAC:"<<sensorParam.macAddress<<"] to sensors Id dictionary";
            return false;
        }

    }

    // Apply profile 
    BioDAQExitStatus exitStatus = ptrBioDAQ->ApplyProfile(ptrSensorIdDictionary);
    if ( exitStatus != BioDAQExitStatus_Success)
    {
        yError()<<"Unable to apply custom profile!";
        return false;
    }
    yInfo()<<"Custom profile applied successfully!";

    return true;
}

bool FreeEmgDevice::FreeEmgDeviceImpl::readLabelToNames(yarp::os::Searchable& config)
{
    // Find group
    yarp::os::Bottle& labelsToNames = config.findGroup("labelToNameGroup");

    // Read label list
    yarp::os::Value& labelListValue = labelsToNames.find("labelList");
    if(labelListValue.isNull()){
        yError() << "Cannot find required parameter labelList";
        return false;
    } else if(!labelListValue.isList()) {
        yError() << "Parameter labelList must be a list, read value is:"<<labelListValue.toString();
        return false;
    }

    yarp::os::Bottle* labelListBottle = labelListValue.asList();
    for(int i=0; i<labelListBottle->size(); i++){
        if(!labelListBottle->get(i).isInt32()){
            yError()<< "Parameter labeList contains the non-integer value:"<<labelListBottle->get(i).toString();
            return false;
        }
        labelList.push_back(labelListBottle->get(i).asInt32());
        if(labelList[i]<0 || labelList[i]>FreeEmg::BIODAQ_EMG_SENSOR_MAX_LABEL){
            yError()<<"Invalid label"<<labelList[i];
            return false;
        }
    }

    // Read sensors names
    yarp::os::Value& sensorNamesValue = labelsToNames.find("sensorNames");
    if(sensorNamesValue.isNull()){
        yError() << "Cannot find required parameter sensorNames";
        return false;
    } else if(!sensorNamesValue.isList()) {
        yError() << "Parameter sensorNames must be a list, read value is:"<<sensorNamesValue.toString();
        return false;
    }

    yarp::os::Bottle* sensorNamesBottle = sensorNamesValue.asList();
    for(int i=0; i<sensorNamesBottle->size(); i++){
        sensorNames.push_back(sensorNamesBottle->get(i).asString());
    }

    // Check size of labels and names lists
    if(sensorNames.size()!=labelList.size()){
        yError()<<"Parameters labelList and sensorsNames must be lists of the same size!";
        return false;
    }
    
    return true;
}

bool FreeEmgDevice::FreeEmgDeviceImpl::readSensorParam(yarp::os::Searchable& config)
{
    // Read the label
    yarp::os::Value& labelValue = config.find("label");
    if(labelValue.isNull()){
        yError() << "Cannot find a valid label parameter";
        return false;
    }

    int label = labelValue.asInt32();
    if(label<FreeEmg::BIODAQ_EMG_SENSOR_MAX_LABEL || label>FreeEmg::BIODAQ_EMG_SENSOR_MAX_LABEL){
        yError() << "Invalid label value:"<<label<<"!";
        return false;
    }

    // Read the color
    yarp::os::Value& colorValue = config.find("color");
    if(colorValue.isNull() || !colorValue.isString()){
        yError() << "Cannot find a valid color parameter!";
        return false;
    }

    OLE_COLOR color;
    if(!FreeEmg::colorFromName(colorValue.asString(), color)){
        yError() << "Unsupported color:" << colorValue.asString();
        return false;
    }

    // Read the sensor identifier
    yarp::os::Value& identifierValue = config.find("identifier");
    if(identifierValue.isNull())
    {
        yError()<<"Cannot find a valid identifier parameter";
        return false;
    }
    
    // Convert param to string
    std::string identifierString = identifierValue.isInt32() ? 
                                   std::to_string(identifierValue.asInt32()) :
                                   identifierValue.asString();

    // Build MAC address from identifier
    std::string macAddressString = FreeEmg::EMG_SENSOR_MAC_PREFIX + identifierString;

    // Check if MAC address is valid
    if(!FreeEmg::validateMacAddress(macAddressString))
    {
        yError() << "Invalid identifier:"<<identifierValue.toString();
        return false;
    }

    yInfo()<<"Found sensor:\nLabel:"<<label<<"\nColor:"<<colorValue.asString()<<"\nMAC:"<<macAddressString;

    // Add sensor param to the lists
    sensorParams.push_back({(unsigned char) label, color, macAddressString});

    return true;
}

bool FreeEmgDevice::FreeEmgDeviceImpl::readProfileParam(yarp::os::Searchable& config)
{
    // Parse param useCustomProfile
    yarp::os::Value& useCustomProfileValue = config.find("useCustomProfile");
    if(useCustomProfileValue.isNull()){
        yError() << LOG_PREFIX << "Cannot find required parameter useCustomProfile";
        return false;
    }
    if(!useCustomProfileValue.isBool()){
        yError() << LOG_PREFIX << "useCustomProfile must be a boolean value!";
        return false;
    }
    useCustomProfile = useCustomProfileValue.asBool();

    // No need to continue if useCustomProfile is false
    if(!useCustomProfile){
        return true;
    }

    // Find group profile
    yarp::os::Bottle& profileGroup = config.findGroup("profile");
    if(profileGroup.isNull()){
        return true;
    }
    
    // Read sensor information
    for(int i = 1; i < profileGroup.size(); i++){
        if(!readSensorParam(profileGroup.get(i))){
            return false;
        }
    }

    return true;
}

bool FreeEmgDevice::FreeEmgDeviceImpl::readParams(yarp::os::Searchable& config)
{   
    // Period of the this device
    if (!(config.check("period") && config.find("period").isFloat64())) {
        yInfo() << LOG_PREFIX << "Using default period:" << period << "s";
    }
    else {
        period = config.find("period").asFloat64();
        yInfo() << LOG_PREFIX << "Using period:" << period << "s";
    }

    // COM ports
    yarp::os::Value& comPortsValue = config.find("COM-ports");
    if (comPortsValue.isNull()) {
        yError()<<"Cannot find required parameter COM-ports!";
        return false;
    }

    if(comPortsValue.isList()){
        yarp::os::Bottle* comPortsBottle = comPortsValue.asList();
        for(int i=0 ; i<comPortsBottle->size() ; i++){
            if(!comPortsBottle->get(i).isInt32()){
                yError() << "Found invalid COM port number" << comPortsBottle->get(i).asString();
                return false;
            }
            comPorts.push_back(comPortsBottle->get(i).asInt32());
        }
    } else if (comPortsValue.isInt32()) {
        comPorts.push_back(comPortsValue.asInt32());
    } else {
        yError()<<"COM-port parameter must be a list or valid integer!";
        return false;
    }

    // Check list of COM ports 
    for(auto & comPort : comPorts){
        if(comPort<=0){
            yError()<<"Found invalid COM port number"<<comPort<<"(non positive number)!";
            return false;
        }
        yInfo()<<"Using COM port:"<<comPort;
    }

    // Parse the profile params
    if(!readProfileParam(config)){
        return false;
    }

    // Read association labels to sensor names
    if(!readLabelToNames(config)){
        return false;
    }

    return true;
}

bool FreeEmgDevice::FreeEmgDeviceImpl::configure(yarp::os::Searchable& config)
{
    // Create the ports List
    IPortListPtr ptrPortList(__uuidof(PortList));

    for(int i = 0; i<comPorts.size(); i++)
    {
        // Create a COM Port with a given number 
        auto uuidofCOMPort = __uuidof(PortCOM);
        IPortCOMPtr ptrCOMPort = IPortCOMPtr(uuidofCOMPort);
        ptrCOMPort->Number = comPorts[i];      // set the COM port number where BioDAQ device is connected (use Windows device manager tool)
        ptrCOMPort->PutBaudRate(BaudRate_baud230400);

        // Get the IPort interface of the ptrCOMPort
        IPort *pPort; 
        ptrCOMPort.QueryInterface(__uuidof(IPort), &pPort);

        // Insert the new COM port
        ptrPortList->Insert(i, pPort);
    }

    // Instantiate the BioDAQ 
    ptrBioDAQ = IBioDAQPtr(__uuidof(BioDAQ));

    // Init the system with the created list of ports
    BioDAQExitStatus exitStatus = ptrBioDAQ->Init(ptrPortList);
    if ( exitStatus != BioDAQExitStatus_Success)
    {
        yError()<<"Unable to initialize BioDAQ device on desired USB COM port";
        return false;
    }
    yInfo()<<"BioDAQ initialized successfully!";

    // Apply profile
    if(!configureProfile())
    {
        return false;
    }

    // Apply protocol
    if(!configureProtocol()){
        return false;
    }

    // Setup data read
    if(!configureDataRead()){
        yError() << LOG_PREFIX << "Cannot initialize the data read!";
        return false;
    }

    return true;
}

bool FreeEmgDevice::FreeEmgDeviceImpl::run()
{
    // Update timestamp as wall-clock time since epoch in ms
    auto timeSinceEpoch = std::chrono::system_clock::now().time_since_epoch(); 
    timeStamp = (double) std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count();

    // Update sensor data
    long channels = ptrBioDAQ->ActiveProtocol->Count();
    for(int channelIdx=0 ; channelIdx<channels; channelIdx++){

        // Get channel and queue size
        bts_biodaq_core::IChannelPtr channel = ptrBioDAQ->ChannelsView->GetItem(channelIdx);
        
        // Skip if channel is inactive
        if(channel->Active==VARIANT_FALSE) continue; //TODO warning?

        float outV = 0.0f; //output

        __int64 sampleIdx = -1;
        while(ptrQueueSink->QueueSize(channelIdx)>=1){

            // Get last sample
            SinkExitStatus exitStatus = ptrQueueSink->Dequeue(
                                                channelIdx, // channel index
                                                &sampleIdx, // sample index 
                                                &outV      // recovery sample value
                                                );

            // TODO what to do in this case?
            if(exitStatus != SinkExitStatus_Success){
                //yWarning() << LOG_PREFIX << "Unable to dequeue for channel"<<channelIdx<<"!";
                sampleIdx = -1;
            }

        };

        if(sampleIdx==-1){
            continue;
        }
        
        double outmV = (double) outV * 1000.0; // data is in V
        
        // Update sensor value
        emgSensorMap[labelList[channelIdx]]->setEmgSignal(outmV);
    }

    return true;
}

bool FreeEmgDevice::FreeEmgDeviceImpl::close()
{
    // Release ptrQueueSink 
    if(ptrQueueSink!=nullptr){
        ptrQueueSink->Release();
        ptrQueueSink = nullptr;
    }

    // Go to IDLE state
    if(ptrBioDAQ!=nullptr){
        if(ptrBioDAQ->State!=BioDAQState_Idle){
            ptrBioDAQ->Reset();
        }
        ptrBioDAQ->Release();
        ptrBioDAQ = nullptr;
    }
    
    return true;
}

FreeEmgDevice::FreeEmgDevice()
    : PeriodicThread(0.01) //default 100Hz
    , m_pImpl{std::make_unique<FreeEmgDeviceImpl>()}
{}

// Destructor
FreeEmgDevice::~FreeEmgDevice() = default;

bool FreeEmgDevice::open(yarp::os::Searchable& config)
{
    // This is needed to use the SDK
    CoInitializeEx(NULL,COINIT_MULTITHREADED);

    // Read configuration parameters
    if(!m_pImpl->readParams(config)){
        return false;
    }

    // Set periodic thread period
    setPeriod(m_pImpl->period);

    if(!m_pImpl->configure(config)){
        return false;
    }
    yInfo()<< LOG_PREFIX <<"Device configured!";

    // Create wearable sensors
    m_pImpl->emgSensorPrefix = getWearableName() + wearable::Separator + sensor::IEmgSensor::getPrefix();
    for(int i=0; i<m_pImpl->sensorNames.size(); i++){
        const std::string& sensorName = m_pImpl->sensorNames[i];
        std::string name = m_pImpl->emgSensorPrefix + sensorName;
        auto sensorPtr = std::make_shared<FreeEmgSensor>( name );
        m_pImpl->emgSensorMap.insert({m_pImpl->labelList[i], sensorPtr});
    }
    yInfo() << LOG_PREFIX << "Created wearable sensors.";

    // Start the PeriodicThread loop
    if (!start()) {
        yError() << LOG_PREFIX << "Failed to start the period thread!";
        return false;
    }

    yInfo() << LOG_PREFIX << "The device is opened successfully.";

    return true;
}

void FreeEmgDevice::run()
{
    m_pImpl->run();
}

bool FreeEmgDevice::close()
{
    // Stop the periodic thread
    stop();

    // Close the device
    if (!m_pImpl->close()) {
        yError() << LOG_PREFIX << "Cannot close correctly the deive.";
        return false;
    }

    // This is needed to use the SDK
    CoUninitialize();

    return true;
}

void FreeEmgDevice::threadRelease() {}

// =========================
// IPreciselyTimed interface
// =========================
yarp::os::Stamp FreeEmgDevice::getLastInputStamp()
{
    // Stamp count should be always zero
    return yarp::os::Stamp(0, m_pImpl->timeStamp);
}

// ---------------------------
// Implement Sensors Methods
// ---------------------------

wearable::WearableName FreeEmgDevice::getWearableName() const
{
    return m_pImpl->wearableName;
}

wearable::WearStatus FreeEmgDevice::getStatus() const
{
    wearable::WearStatus status = wearable::WearStatus::Ok;

    for (const auto& s : getAllSensors()) {
        if (s->getSensorStatus() != sensor::SensorStatus::Ok) {
            status = wearable::WearStatus::Error;
            // TO CHECK
            break;
        }
    }
    // Default return status is Ok
    return status;
}

wearable::TimeStamp FreeEmgDevice::getTimeStamp() const
{
    return {m_pImpl->timeStamp, 0};
}

wearable::SensorPtr<const wearable::sensor::ISensor>
FreeEmgDevice::getSensor(const wearable::sensor::SensorName name) const
{
    wearable::VectorOfSensorPtr<const wearable::sensor::ISensor> sensors = getAllSensors();
    for (const auto& s : sensors) {
        if (s->getSensorName() == name) {
            return s;
        }
    }
    yWarning() << LOG_PREFIX << "User specified name <" << name << "> not found";
    return nullptr;
}

wearable::VectorOfSensorPtr<const wearable::sensor::ISensor>
FreeEmgDevice::getSensors(const wearable::sensor::SensorType aType) const
{
    wearable::VectorOfSensorPtr<const wearable::sensor::ISensor> outVec;
    switch (aType) {
        case sensor::SensorType::EmgSensor: {
            outVec.reserve(m_pImpl->emgSensorMap.size());
            for (const auto& emgSensor : m_pImpl->emgSensorMap)
                outVec.push_back(static_cast<SensorPtr<sensor::ISensor>>(emgSensor.second));
            break;
        }
        default: {
            return {};
        }
    }

    return outVec;
}

wearable::VectorOfElementPtr<const wearable::actuator::IActuator>
FreeEmgDevice::getActuators(const wearable::actuator::ActuatorType aType) const
{
    return {};
}

wearable::SensorPtr<const wearable::sensor::IEmgSensor>
FreeEmgDevice::getEmgSensor(const wearable::sensor::SensorName name) const
{
    // Return a shared point to the required sensor
    for (const auto& emgSensor : m_pImpl->emgSensorMap)
    {
        if(emgSensor.second->getSensorName()==name)
        {
            return static_cast<SensorPtr<sensor::IEmgSensor>>(emgSensor.second);
        }
    }
    
    return nullptr;
}
