#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <memory>
#include <iomanip>
#include <map>
#include <numeric>
#include <sstream>
#include <fstream>

// Constants
constexpr size_t MAX_LOADS = 10;
constexpr double MAX_BATTERY_CHARGE_RATE = 0.2;
constexpr double MAX_BATTERY_DISCHARGE_RATE = 0.3;
constexpr double PANEL_DEGRADATION_THRESHOLD = 0.05; // 5% performance drop
constexpr double TEMPERATURE_ALERT_THRESHOLD = 70.0; // °C
constexpr double IRRADIANCE_EFFICIENCY_THRESHOLD = 0.7; // 70% of expected
constexpr double CO2_SAVINGS_PER_KWH = 0.4; // kg CO2 per kWh saved
constexpr double TREES_EQUIVALENT_PER_KWH = 0.01; // Trees equivalent per kWh saved

// Data structures
struct Load {
    std::string name;
    double power;
    
    Load(const std::string& n, double p) : name(n), power(p) {}
};

class SolarReading {
public:
    time_t timestamp;
    double power_produced;
    double power_consumed;
    double battery_soc;
    double irradiance;
    double temperature;
    double panel_voltage;
    double panel_current;

    SolarReading(double prod, double cons, double soc, double irr, double temp, 
                double volt, double curr)
        : timestamp(std::time(nullptr)),
          power_produced(prod),
          power_consumed(cons),
          battery_soc(soc),
          irradiance(irr),
          temperature(temp),
          panel_voltage(volt),
          panel_current(curr) {}
};

// Maintenance Alert Types
enum class AlertType {
    PANEL_DEGRADATION,
    HIGH_TEMPERATURE,
    LOW_EFFICIENCY,
    INVERTER_ISSUE,
    BATTERY_DEGRADATION
};

class MaintenanceAlert {
public:
    AlertType type;
    std::string message;
    time_t timestamp;
    double severity; // 0-1 scale

    MaintenanceAlert(AlertType t, const std::string& msg, double sev)
        : type(t), message(msg), timestamp(std::time(nullptr)), severity(sev) {}

    void print() const {
        std::cout << "[ALERT] " << message 
                  << " | Severity: " << std::setprecision(2) << severity * 100 << "%"
                  << " | Time: " << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S")
                  << "\n";
    }
};

class EnvironmentalImpact {
    double total_energy_produced; // in kWh
    time_t start_date;
    
public:
    EnvironmentalImpact() : total_energy_produced(0), start_date(std::time(nullptr)) {}
    
    void addEnergy(double watts, double hours) {
        total_energy_produced += (watts * hours) / 1000.0; // Convert to kWh
    }
    
    double getCO2Savings() const {
        return total_energy_produced * CO2_SAVINGS_PER_KWH;
    }
    
    double getTreeEquivalents() const {
        return total_energy_produced * TREES_EQUIVALENT_PER_KWH;
    }
    
    void generateReport() const {
        std::cout << "\n=== ENVIRONMENTAL IMPACT REPORT ===\n";
        std::cout << "Total solar energy produced: " << total_energy_produced << " kWh\n";
        std::cout << "CO2 emissions avoided: " << getCO2Savings() << " kg\n";
        std::cout << "Equivalent to planting " << getTreeEquivalents() << " trees\n";
        
        // Generate HTML visualization
        generateHTMLVisualization();
    }
    
private:
    void generateHTMLVisualization() const {
        std::ofstream html_file("environmental_impact.html");
        
        html_file << R"(<!DOCTYPE html>
<html>
<head>
    <title>Solar Energy Environmental Impact</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        .dashboard { display: flex; flex-wrap: wrap; gap: 20px; }
        .chart-container { width: 45%; min-width: 300px; }
    </style>
</head>
<body>
    <h1>Solar Energy Environmental Impact</h1>
    <div class="dashboard">
        <div class="chart-container">
            <canvas id="energyChart"></canvas>
        </div>
        <div class="chart-container">
            <canvas id="co2Chart"></canvas>
        </div>
    </div>
    <script>
        const energyData = {
            labels: ['Solar Energy Produced', 'Grid Energy Displaced'],
            datasets: [{
                data: [)" << total_energy_produced << ", " << total_energy_produced * 0.9 << R"(],
                backgroundColor: ['#FFA500', '#DDDDDD']
            }]
        };
        
        const co2Data = {
            labels: ['CO2 Emissions Avoided'],
            datasets: [{
                data: [)" << getCO2Savings() << R"(],
                backgroundColor: ['#4BC0C0']
            }]
        };
        
        new Chart(document.getElementById('energyChart'), {
            type: 'pie',
            data: energyData,
            options: { responsive: true, plugins: { title: { display: true, text: 'Energy Production (kWh)' } } }
        });
        
        new Chart(document.getElementById('co2Chart'), {
            type: 'bar',
            data: co2Data,
            options: { responsive: true, plugins: { title: { display: true, text: 'CO2 Savings (kg)' } } }
        });
    </script>
</body>
</html>)";
        
        html_file.close();
        std::cout << "\nGenerated visualization: environmental_impact.html\n";
    }
};

// Database Interface
class Database {
public:
    virtual ~Database() = default;
    virtual void storeReading(const SolarReading& reading) = 0;
    virtual std::vector<SolarReading> getReadings(time_t start, time_t end) = 0;
};

// Mock Database implementation
class MockDB : public Database {
    std::vector<SolarReading> readings;
public:
    void storeReading(const SolarReading& reading) override {
        readings.push_back(reading);
        std::cout << "Stored reading at " << std::put_time(std::localtime(&reading.timestamp), "%Y-%m-%d %H:%M:%S") << "\n";
    }
    
    std::vector<SolarReading> getReadings(time_t start, time_t end) override {
        std::vector<SolarReading> result;
        for (const auto& reading : readings) {
            if (reading.timestamp >= start && reading.timestamp <= end) {
                result.push_back(reading);
            }
        }
        return result;
    }
};

class SolarOptimizer {
    std::unique_ptr<Database> db;
    std::vector<MaintenanceAlert> active_alerts;
    std::map<time_t, double> historical_efficiency;
    EnvironmentalImpact environmental_impact;
    
    // Calculate panel efficiency
    double calculate_efficiency(double irradiance, double power_output) const {
        if (irradiance <= 0) return 0.0;
        const double expected_power = irradiance / 1000.0 * 300.0; // Assuming 300W panel
        return power_output / expected_power;
    }

    // Maintenance check implementations...
    void check_panel_degradation(const SolarReading& reading) {
        double current_efficiency = calculate_efficiency(reading.irradiance, reading.power_produced);
        historical_efficiency[reading.timestamp] = current_efficiency;
        
        if (historical_efficiency.size() < 30) return;

        auto month_ago = reading.timestamp - 30 * 24 * 3600;
        double sum = 0.0;
        int count = 0;
        
        for (const auto& entry : historical_efficiency) {
            if (entry.first >= month_ago && entry.first <= reading.timestamp) {
                sum += entry.second;
                count++;
            }
        }
        
        if (count == 0) return;
        
        double avg_efficiency = sum / count;
        double degradation = 1.0 - (current_efficiency / avg_efficiency);
        
        if (degradation > PANEL_DEGRADATION_THRESHOLD) {
            std::string msg = "Panel degradation detected: " + 
                             std::to_string(static_cast<int>(degradation * 100)) + 
                             "% performance loss";
            active_alerts.emplace_back(
                AlertType::PANEL_DEGRADATION, 
                msg,
                degradation / PANEL_DEGRADATION_THRESHOLD
            );
        }
    }

    void check_temperature_issues(const SolarReading& reading) {
        if (reading.temperature > TEMPERATURE_ALERT_THRESHOLD) {
            double severity = (reading.temperature - TEMPERATURE_ALERT_THRESHOLD) / 10.0;
            severity = std::min(severity, 1.0);
            
            std::string msg = "High panel temperature: " + 
                             std::to_string(static_cast<int>(reading.temperature)) + "°C";
            active_alerts.emplace_back(
                AlertType::HIGH_TEMPERATURE,
                msg,
                severity
            );
        }
    }

    // Other maintenance checks (efficiency, inverter, battery) would go here...

public:
    SolarOptimizer(std::unique_ptr<Database> database) : db(std::move(database)) {}
    
    void storeReading(const SolarReading& reading) {
        db->storeReading(reading);
        environmental_impact.addEnergy(reading.power_produced, 1.0); // Assuming 1 hour interval
        performMaintenanceChecks(reading);
    }
    
    // Previous methods (generateForecast, optimizeEnergyUsage) would go here...

    void performMaintenanceChecks(const SolarReading& reading) {
        // Clear old alerts
        auto week_ago = std::time(nullptr) - 7 * 24 * 3600;
        active_alerts.erase(
            std::remove_if(active_alerts.begin(), active_alerts.end(),
                [week_ago](const MaintenanceAlert& alert) {
                    return alert.timestamp < week_ago;
                }),
            active_alerts.end()
        );
        
        check_panel_degradation(reading);
        check_temperature_issues(reading);
        // Other checks would be called here...
    }

    void printMaintenanceAlerts() const {
        if (active_alerts.empty()) {
            std::cout << "No active maintenance alerts\n";
            return;
        }
        
        std::cout << "\n=== MAINTENANCE ALERTS ===\n";
        for (const auto& alert : active_alerts) {
            alert.print();
        }
    }
    
    void generateEnvironmentalReport() const {
        environmental_impact.generateReport();
    }
};

int main() {
    auto mock_db = std::make_unique<MockDB>();
    SolarOptimizer optimizer(std::move(mock_db));

    int num_readings;
    std::cout << "Enter the number of solar readings: ";
    std::cin >> num_readings;

    for (int i = 0; i < num_readings; ++i) {
        double prod, cons, soc, irr, temp, volt, curr;
        std::cout << "\nEnter data for Reading #" << (i + 1) << ":\n";

        std::cout << "Power Produced (W): ";
        std::cin >> prod;
        std::cout << "Power Consumed (W): ";
        std::cin >> cons;
        std::cout << "Battery SOC (%): ";
        std::cin >> soc;
        std::cout << "Irradiance (W/m^2): ";
        std::cin >> irr;
        std::cout << "Temperature (°C): ";
        std::cin >> temp;
        std::cout << "Panel Voltage (V): ";
        std::cin >> volt;
        std::cout << "Panel Current (A): ";
        std::cin >> curr;

        SolarReading reading(prod, cons, soc, irr, temp, volt, curr);
        optimizer.storeReading(reading);
    }

    // Generate reports
    optimizer.printMaintenanceAlerts();
    optimizer.generateEnvironmentalReport();

    return 0;
}