#include "CsvMetricWriter.hpp"
#include "VisualiserManager.hpp"
#include "ProtocolVisualisationState.hpp"
#include "../../Visualisation/Device/Device.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <memory>
#include <mutex>
#include <magic_enum.hpp> // For enum to string conversion

void CsvMetricWriter::writeNetworkMetricsToCsv(const std::string& filename,
                                               const VisualiserManager& manager,
                                               const ProtocolVisualisationState& state) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    // General network info
    const auto& devices = manager.getDevices();
    out << "Communication Mode," << state.communicationMode << '\n';
    out << "Number of Nodes," << devices.size() << '\n';
    out << "Energy Expenditure," << state.energyExp << '\n';
    out << "Retransmissions," << state.retransmissions << '\n';
    out << "Total Data Packets Sent," << state.totalDataPacketsSent << "\n\n";

    out << "ID,Class,HopCount,"
    << "Packets Sent,Retransmissions,PDR,"
    << "ListenData,TransmitData,ListenAck,TransmitAck,TotalEnergy\n";


    std::scoped_lock lock(manager.getDevicesMutex());
    for (const auto& [id, devicePtr]  : devices) {
        const Device& dev = *devicePtr;
        const auto& metrics = dev.metrics;

        out << dev.getNodeId() << ','
            << magic_enum::enum_name(dev.getClass()) << ','
            << dev.getHopCount() << ','
            << metrics.getPacketsSent() << ','
            << metrics.getRetransmissions() << ','
            << metrics.getPacketDeliveryRatio() << ','
            << metrics.getListeningDataSlots() << ','
            << metrics.getTransmittingDataSlots() << ','
            << metrics.getListeningAckSlots() << ','
            << metrics.getTransmittingAckSlots() << ','
            << metrics.getTotalEnergySlots() << '\n';

    }

    out.close();
}
