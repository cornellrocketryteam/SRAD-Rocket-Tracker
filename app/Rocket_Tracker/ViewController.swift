//
//  ViewController.swift
//  Rocket_Tracker
//
//  Created by Ronit Totlani on 3/5/25.
//

import UIKit
import Foundation
import CoreBluetooth
import MapKit
import CoreLocation

// Initialize global variables
var curPeripheral: CBPeripheral?
var rxCharacteristics: [CBCharacteristic] = []
var txCharacteristics: [CBCharacteristic] = []
var BLE_uuid_rx: [CBUUID] = []
var BLE_uuid_tx: [CBUUID] = []
var latitude:Int = 0
var longitude:Int = 0
let pin = MKPointAnnotation()
var done: Int = 0
var firstRecived: Bool = false
var currentLocation: CLLocation?
var latcurr: CLLocationDegrees = 0
var longcurr: CLLocationDegrees = 0

class ViewController: UIViewController, CBCentralManagerDelegate, CBPeripheralDelegate, CLLocationManagerDelegate {
    
    // Variable Initializations
    var centralManager: CBCentralManager!
    var rssiList = [NSNumber]()
    var peripheralList: [CBPeripheral] = []
    var characteristicList = [String: CBCharacteristic]()
    var characteristicValue = [CBUUID: NSData]()
    
    let BLE_Service_UUID = CBUUID.init(string: "00000001-0000-0715-2006-853A52A41A44")
    let BLE_Lat_uuid_Rx = CBUUID.init(string: "00000002-0000-0715-2006-853A52A41A44")
    let BLE_Lat_uuid_Tx  = CBUUID.init(string: "00000002-0000-0715-2006-853A52A41A44")
    let BLE_Long_uuid_Rx = CBUUID.init(string: "00000003-0000-0715-2006-853A52A41A44")
    let BLE_Long_uuid_Tx  = CBUUID.init(string: "00000003-0000-0715-2006-853A52A41A44")
    let BLE_PT3_uuid_Rx = CBUUID.init(string: "00000004-0000-0715-2006-853A52A41A44")
    let BLE_PT3_uuid_Tx  = CBUUID.init(string: "00000004-0000-0715-2006-853A52A41A44")
    let BLE_PT4_uuid_Rx = CBUUID.init(string: "00000005-0000-0715-2006-853A52A41A44")
    let BLE_PT4_uuid_Tx  = CBUUID.init(string: "00000005-0000-0715-2006-853A52A41A44")
    let BLE_MAV_uuid_Rx = CBUUID.init(string: "00000006-0000-0715-2006-853A52A41A44")
    let BLE_MAV_uuid_Tx  = CBUUID.init(string: "00000006-0000-0715-2006-853A52A41A44")
    let BLE_SV_uuid_Rx = CBUUID.init(string: "00000007-0000-0715-2006-853A52A41A44")
    let BLE_SV_uuid_Tx  = CBUUID.init(string: "00000007-0000-0715-2006-853A52A41A44")
    let BLE_FM_uuid_Rx = CBUUID.init(string: "00000008-0000-0715-2006-853A52A41A44")
    let BLE_FM_uuid_Tx  = CBUUID.init(string: "00000008-0000-0715-2006-853A52A41A44")
    let BLE_PT_uuid_Rx = CBUUID.init(string: "00000009-0000-0715-2006-853A52A41A44")
    let BLE_PT_uuid_Tx  = CBUUID.init(string: "00000009-0000-0715-2006-853A52A41A44")
    
    @IBOutlet weak var arrowBtn: UIImageView!
    @IBOutlet weak var connectStatusLbl: UILabel!
    @IBOutlet weak var mapView: MKMapView!
    @IBOutlet weak var distanceButton: UIButton!
    @IBOutlet weak var fmButton: UIButton!
    @IBOutlet weak var pt3Button: UIButton!
    @IBOutlet weak var pt4Button: UIButton!
    @IBOutlet weak var mavButton: UIButton!
    @IBOutlet weak var svButton: UIButton!
    @IBOutlet weak var ptLbl: UILabel!
    @IBOutlet weak var refreshBtn: UIBarButtonItem!
    @IBOutlet weak var coordsLbl: UILabel!
    let manager = CLLocationManager()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let tripleTapGesture = UITapGestureRecognizer(target: self, action: #selector(handleTripleTap(_:)))
        tripleTapGesture.numberOfTapsRequired = 3  // triple-tap
        mapView.addGestureRecognizer(tripleTapGesture)
        
        manager.desiredAccuracy = kCLLocationAccuracyBest
        manager.delegate = self
        manager.requestWhenInUseAuthorization()
        manager.startUpdatingLocation()
        manager.startUpdatingHeading()
        connectStatusLbl.text = "Disconnected"
        connectStatusLbl.textColor = UIColor.red
        ptLbl.text = "Have Not Recived"
        distanceButton.setTitle("Distance to LV: ", for: .normal)
        fmButton.setTitle("Flightmode: ", for: .normal)
        pt3Button.setTitle("PT3: ", for: .normal)
        pt4Button.setTitle("PT4: ", for: .normal)
        mavButton.setTitle("MAV: ", for: .normal)
        svButton.setTitle("SV: ", for: .normal)
        BLE_uuid_rx += [BLE_Lat_uuid_Rx, BLE_Long_uuid_Rx, BLE_PT3_uuid_Rx, BLE_PT4_uuid_Rx, BLE_MAV_uuid_Rx, BLE_SV_uuid_Rx, BLE_FM_uuid_Rx, BLE_PT_uuid_Rx]
        BLE_uuid_tx += [BLE_Lat_uuid_Tx, BLE_Long_uuid_Tx, BLE_PT3_uuid_Tx, BLE_PT4_uuid_Tx, BLE_MAV_uuid_Tx, BLE_SV_uuid_Tx, BLE_FM_uuid_Tx, BLE_PT_uuid_Tx]
        centralManager = CBCentralManager(delegate: self, queue: nil)
        refreshBtn.target = self
        refreshBtn.action = #selector(refreshBtnClicked)
        
        // Add a tap gesture recognizer to all buttons
        addTapGesture(to: fmButton, action: #selector(fmButtonTapped))
        addTapGesture(to: pt3Button, action: #selector(pt3ButtonTapped))
        addTapGesture(to: pt4Button, action: #selector(pt4ButtonTapped))
        addTapGesture(to: mavButton, action: #selector(mavButtonTapped))
        addTapGesture(to: svButton, action: #selector(svButtonTapped))
        addTapGesture(to: distanceButton, action: #selector(distanceButtonTapped))
    }
    
    // Helper function to add tap gesture recognizers
    func addTapGesture(to button: UIButton, action: Selector) {
        let tapGesture = UITapGestureRecognizer(target: self, action: action)
        button.addGestureRecognizer(tapGesture)
        button.isUserInteractionEnabled = true // Enable user interaction
    }
    
    @objc func fmButtonTapped() {
        requestCharacteristicValue(for: BLE_FM_uuid_Rx)
    }

    @objc func pt3ButtonTapped() {
        requestCharacteristicValue(for: BLE_PT3_uuid_Rx)
    }

    @objc func pt4ButtonTapped() {
        requestCharacteristicValue(for: BLE_PT4_uuid_Rx)
    }

    @objc func mavButtonTapped() {
        requestCharacteristicValue(for: BLE_MAV_uuid_Rx)
    }

    @objc func svButtonTapped() {
        requestCharacteristicValue(for: BLE_SV_uuid_Rx)
    }

    @objc func distanceButtonTapped() {
        // Request both latitude and longitude characteristics
        requestCharacteristicValue(for: BLE_Lat_uuid_Rx)
        requestCharacteristicValue(for: BLE_Long_uuid_Rx)
    }

    // Helper function to request characteristic value
    func requestCharacteristicValue(for uuid: CBUUID) {
        // Check if the peripheral is connected
        guard let peripheral = curPeripheral, peripheral.state == .connected else {
            print("Bluetooth is not connected")
            return
        }
        
        // Find the characteristic with the specified UUID
        if let characteristic = rxCharacteristics.first(where: { $0.uuid == uuid }) {
            // Manually read the value for the characteristic
            peripheral.readValue(for: characteristic)
            print("Manually requested value for characteristic: \(uuid)")
        } else {
            print("Characteristic not found for UUID: \(uuid)")
        }
    }
    
    // This function is called right after the view is loaded onto the screen
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        // Reset the peripheral connection with the app
        if curPeripheral != nil {
            centralManager?.cancelPeripheralConnection(curPeripheral!)
        }
        print("View Cleared")
    }
    
    // This function is called right before view disappears from screen
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        print("Stop Scanning")
        
        // Central Manager object stops the scanning for peripherals
        centralManager?.stopScan()
    }
    
    // Called when manager's state is changed
    // Required method for setting up centralManager object
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        
        // If manager's state is "poweredOn", that means Bluetooth has been enabled
        // in the app. We can begin scanning for peripherals
        if central.state == CBManagerState.poweredOn {
            print("Bluetooth Enabled")
            startScan()
        }
        
        // Else, Bluetooth has NOT been enabled, so we display an alert message to the screen
        // saying that Bluetooth needs to be enabled to use the app
        else {
            print("Bluetooth Disabled- Make sure your Bluetooth is turned on")

            let alertVC = UIAlertController(title: "Bluetooth is not enabled",
                                            message: "Make sure that your bluetooth is turned on",
                                            preferredStyle: UIAlertController.Style.alert)
            
            let action = UIAlertAction(title: "ok",
                                       style: UIAlertAction.Style.default,
                                       handler: { (action: UIAlertAction) -> Void in
                                                self.dismiss(animated: true, completion: nil)
                                                })
            alertVC.addAction(action)
            self.present(alertVC, animated: true, completion: nil)
        }
    }
    // Start scanning for peripherals
    func startScan() {
        print("Now Scanning...")
        print("Service ID Search: \(BLE_Service_UUID)")
        
        // Make an empty list of peripherals that were found
        peripheralList = []
        
        centralManager?.scanForPeripherals(withServices: [BLE_Service_UUID],
                                           options: [CBCentralManagerScanOptionAllowDuplicatesKey:false])
    }
    
// Cancel scanning for peripheral
    func cancelScan() {
        self.centralManager?.stopScan()
        print("Scan Stopped")
        print("Number of Peripherals Found: \(peripheralList.count)")
    }

    // Called when a peripheral is found.
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        // The peripheral that was just found is stored in a variable and
        // is added to a list of peripherals. Its rssi value is also added to a list
        curPeripheral = peripheral
        self.peripheralList.append(peripheral)
        self.rssiList.append(RSSI)
        peripheral.delegate = self

        // Connect to the peripheral if it exists / has services
        if curPeripheral != nil {
            centralManager?.connect(curPeripheral!, options: nil)
        }
    }
    
    // Restore the Central Manager delegate if something goes wrong
    func restoreCentralManager() {
        centralManager?.delegate = self
    }
    
    // Called when app successfully connects with the peripheral
    // Use this method to set up the peripheral's delegate and discover its services
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("-------------------------------------------------------")
        print("Connection complete")
        print("Peripheral info: \(String(describing: curPeripheral))")
        
        // Stop scanning because we found the peripheral we want
        cancelScan()
        
        // Set up peripheral's delegate
        peripheral.delegate = self
        
        // Only look for services that match our specified UUID
        peripheral.discoverServices([BLE_Service_UUID])
    }
    
    // Called when the central manager fails to connect to a peripheral
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        
        // Print error message to console for debugging purposes
        if error != nil {
            print("Failed to connect to peripheral")
            return
        }
    }
    
    // Called when the central manager disconnects from the peripheral
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("Disconnected")
        connectStatusLbl.text = "Disconnected"
        connectStatusLbl.textColor = UIColor.red
        ptLbl.text = ""
        
        startScan()
    }

    // Called when the correct peripheral's services are discovered
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        print("-------------------------------------------------------")
        
        // Check for any errors in discovery
        if ((error) != nil) {
            print("Error discovering services: \(error!.localizedDescription)")
            return
        }

        // Store the discovered services in a variable. If no services are there, return
        guard let services = peripheral.services else {
            return
        }
        
        // Print to console for debugging purposes
        print("Discovered Services: \(services)")

        // For every service found...
        for service in services {
            
            // If service's UUID matches with our specified one...
            if service.uuid == BLE_Service_UUID {
                print("Service found")
                connectStatusLbl.text = "Connected"
                connectStatusLbl.textColor = UIColor.green
                
                // Search for the characteristics of the service
                peripheral.discoverCharacteristics(nil, for: service)
            }
        }
    }
    
    // Called when the characteristics we specified are discovered
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        print("-------------------------------------------------------")
        
        // Check if there was an error
        if ((error) != nil) {
            print("Error discovering services: \(error!.localizedDescription)")
            return
        }
        
        // Store the discovered characteristics in a variable. If no characteristics, then return
        guard let characteristics = service.characteristics else {
            return
        }
        
        // Print to console for debugging purposes
        print("Found \(characteristics.count) characteristics!")
        
        // For every characteristic found...
        for characteristic in characteristics {
            
            if BLE_uuid_rx.contains(characteristic.uuid) {
                rxCharacteristics.append(characteristic)
                
                // Subscribe to the this particular characteristic
                // This will also call didUpdateNotificationStateForCharacteristic
                // method automatically
                peripheral.setNotifyValue(true, for: rxCharacteristics.last!)
                peripheral.readValue(for: characteristic)
                print("Rx Characteristic: \(characteristic.uuid)")
            }
            
            // If characteritstic's UUID matches with our specified one for Tx...
            if BLE_uuid_tx.contains(characteristic.uuid){
                txCharacteristics.append(characteristic)
                print("Tx Characteristic: \(characteristic.uuid)")
            }
            
            // Find descriptors for each characteristic
            peripheral.discoverDescriptors(for: characteristic)
        }
    }

    // Sets up notifications to the app from the Feather
    // Calls didUpdateValueForCharacteristic() whenever characteristic's value changes
    func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
        print("*******************************************************")

        // Check if subscription was successful
        if (error != nil) {
            print("Error changing notification state:\(String(describing: error?.localizedDescription))")

        } else {
            print("Characteristic's value subscribed")
        }

        // Print message for debugging purposes
        if (characteristic.isNotifying) {
            print ("Subscribed. Notification has begun for: \(characteristic.uuid)")
        }
    }
    
    // Called when peripheral.readValue(for: characteristic) is called
    // Also called when characteristic value is updated in
    // didUpdateNotificationStateFor() method
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic,
                    error: Error?) {
        
        // If characteristic is correct, read its value and save it to a string.
        // Else, return
        guard rxCharacteristics.contains(characteristic),
        let characteristicValue = characteristic.value,
        let receivedString = NSString(data: characteristicValue,
                                      encoding: String.Encoding.utf8.rawValue)
        else { return }
        
        switch characteristic.uuid {
        case BLE_Lat_uuid_Rx:
            if let latValue = Int(receivedString as String) {
                latitude = latValue
                renderLoc(currlat: latitude, currlong: longitude)
                updateDistanceToPin()
                if (firstRecived) {
                    let LatDeci=Double(latitude)/1000000
                    let LongDeci=Double(longitude)/1000000
                    coordsLbl.text = "Coordinates: (\(String(format: "%.6f", LatDeci)),\(String(format: "%.6f", LongDeci)))"
                }
                else {
                    coordsLbl.text = "Coordinates: N/A"
                }
            }
            else {print("Error with Parsing Latitiude")}
        case BLE_Long_uuid_Rx:
            if let longValue = Int(receivedString as String) {
                longitude = longValue
                renderLoc(currlat: latitude, currlong: longitude)
                updateDistanceToPin()
                if (firstRecived) {
                    let LatDeci=Double(latitude)/1000000
                    let LongDeci=Double(longitude)/1000000
                    coordsLbl.text = "Coordinates: (\(String(format: "%.6f", LatDeci)),\(String(format: "%.6f", LongDeci)))"
                }
                else {
                    coordsLbl.text = "Coordinates: N/A"
                }
            }
            else {print("Error with Parsing longitude")}
        case BLE_PT3_uuid_Rx:
            // Convert the received string to a Float
            if let floatValue = Float(receivedString as String) {
                // Format the float to 3 decimal places
                let formattedValue = String(format: "%.3f", floatValue)
                // Update the button's title
                pt3Button.setTitle("PT3: \(formattedValue)", for: .normal)
            } else {
                // Handle the case where the string cannot be converted to a Float
                pt3Button.setTitle("PT3: Invalid Value", for: .normal)
            }
        case BLE_PT4_uuid_Rx:
            // Convert the received string to a Float
            if let floatValue = Float(receivedString as String) {
                // Format the float to 3 decimal places
                let formattedValue = String(format: "%.3f", floatValue)
                // Update the button's title
                pt4Button.setTitle("PT4: \(formattedValue)", for: .normal)
            } else {
                // Handle the case where the string cannot be converted to a Float
                pt4Button.setTitle("PT4: Invalid Value", for: .normal)
            }
        case BLE_MAV_uuid_Rx:
            if let intValue = Int(receivedString as String) {
                // Map 1 to "Open" and 0 to "Close"
                let status = (intValue == 1) ? "Opened" : "Closed"
                // Update the button's title
                mavButton.setTitle("MAV: \(status)", for: .normal)
            } else {
                // Handle the case where the string cannot be converted to an Int
                mavButton.setTitle("MAV: Invalid Value", for: .normal)
            }
        case BLE_SV_uuid_Rx:
            if let intValue = Int(receivedString as String) {
                // Map 1 to "Open" and 0 to "Close"
                let status = (intValue == 1) ? "Opened" : "Closed"
                // Update the button's title
                svButton.setTitle("SV: \(status)", for: .normal)
            } else {
                // Handle the case where the string cannot be converted to an Int
                svButton.setTitle("SV: Invalid Value", for: .normal)
            }
        case BLE_FM_uuid_Rx:
            if let intValue = Int(receivedString as String) {
                switch intValue
                {
                case 0:
                    fmButton.setTitle("FM: Startup", for: .normal)
                case 1:
                    fmButton.setTitle("FM: Standbye", for: .normal)
                case 2:
                    fmButton.setTitle("FM: Ascent", for: .normal)
                case 3:
                    fmButton.setTitle("FM: Drogue Deployed", for: .normal)
                case 4:
                    fmButton.setTitle("FM: Main Deployed", for: .normal)
                default:
                    fmButton.setTitle("FM: Invalid", for: .normal)
                }
            }
            else {fmButton.setTitle("FM: Invalid", for: .normal)}
        case BLE_PT_uuid_Rx:
            if let intValue = Int(receivedString as String) {
                if (intValue == 0 && !firstRecived) {
                    ptLbl.text = "No Packet Has Been Recvied Yet"
                }
                else if (intValue < 300) {
                    firstRecived = true
                    ptLbl.text = "Time Since The Last Packet: " + (receivedString as String)
                    let LatDeci=Double(latitude)/1000000
                    let LongDeci=Double(longitude)/1000000
                    coordsLbl.text = "Coordinates: (\(String(format: "%.6f", LatDeci)),\(String(format: "%.6f", LongDeci)))"
                }
                else {
                    firstRecived = true
                    ptLbl.text = "Time Since The Last Packet: > 5 Minutes"}
            } else {
                // Handle the case where the string cannot be converted to an Int
                ptLbl.text = "Time Since The Last Packet: Invalid Value"
            }
            
        default:
            print("Unknown characteristic: \(characteristic.uuid)")
        }
        
        NotificationCenter.default.post(name:NSNotification.Name(rawValue: "Notify"), object: self)
    }
    // Called when app wants to send a message to peripheral
    func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        guard error == nil else {
            print("Error discovering services: error")
            return
        }
        print("Message sent")
    }

    // Called when descriptors for a characteristic are found
    func peripheral(_ peripheral: CBPeripheral, didDiscoverDescriptorsFor characteristic: CBCharacteristic, error: Error?) {
        
        // Print for debugging purposes
        print("*******************************************************")
        if error != nil {
            print("\(error.debugDescription)")
            return
        }
        
        // Store descriptors in a variable. Return if nonexistent.
        guard let descriptors = characteristic.descriptors else { return }
            
        // For every descriptor, print its description for debugging purposes
        descriptors.forEach { descript in
            print("function name: DidDiscoverDescriptorForChar \(String(describing: descript.description))")
        }
    }
    
    @objc func refreshBtnClicked() {
        // Check if the peripheral is connected
        guard let peripheral = curPeripheral, peripheral.state == .connected else {
            print("Bluetooth is not connected")
            return
        }
        
        // Manually read values for all characteristics
        for characteristic in rxCharacteristics {
            peripheral.readValue(for: characteristic)
        }
        
        print("Manually fetched all Bluetooth values")
        
    }
    
    func renderLoc(currlat: Int, currlong: Int) {
        mapView.removeAnnotation(pin)
        let lat = CLLocationDegrees(currlat) / 1000000.0
        let long = CLLocationDegrees(currlong) / 1000000.0
        latcurr = lat
        longcurr = long
        let launchVehicle = CLLocationCoordinate2D(latitude: lat, longitude: long)
        if (done<=4){
            let span = MKCoordinateSpan(latitudeDelta: 0.1, longitudeDelta: 0.1)
            let region = MKCoordinateRegion(center: launchVehicle, span: span)
            mapView.setRegion(region, animated: true)
            done += 1
        }
        pin.coordinate = launchVehicle
        if firstRecived {
            mapView.addAnnotation(pin)
        }
        
        print("Placed Marker")
        
    }
    func locationManager(_ manager: CLLocationManager, didUpdateLocations locations: [CLLocation]) {
        if let location = locations.last {
            currentLocation = location
            updateArrowDirection()
        }
    }
    
    func locationManager(_ manager: CLLocationManager, didUpdateHeading newHeading: CLHeading) {
        // Update the arrow's rotation whenever the heading changes
        updateArrowDirection()
    }
    
    func calculateBearing(from source: CLLocationCoordinate2D, to destination: CLLocationCoordinate2D) -> Double {
        let lat1 = (source.latitude * .pi / 180)
        let lon1 = (source.longitude * .pi / 180)
        let lat2 = (destination.latitude * .pi / 180)
        let lon2 = (destination.longitude * .pi / 180)

        let dLon = lon2 - lon1

        let y = sin(dLon) * cos(lat2)
        let x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon)
        let radiansBearing = atan2(y, x)

        return (radiansBearing * 180 / .pi)
    }
    
    func updateArrowDirection() {
        guard let currentLocation = currentLocation else { return }

        // Get the rocket's location
        let rocketLocation = CLLocationCoordinate2D(latitude: pin.coordinate.latitude, longitude: pin.coordinate.longitude)

        // Calculate the bearing to the rocket
        let bearing = calculateBearing(from: currentLocation.coordinate, to: rocketLocation)

        // Get the device's current heading (compass direction)
        guard let heading = manager.heading?.trueHeading else { return }

        // Calculate the angle to rotate the arrow
        let angle = ((bearing - heading) * .pi / 180)

        // Apply the rotation to the arrowBtn
        arrowBtn.transform = CGAffineTransform(rotationAngle: CGFloat(angle))
    }
    
    
    func updateDistanceToPin() {
        // Ensure current location and pin are available
        guard let currentLocation = currentLocation else {
            print("Current location not available")
            return
        }
        
        // Get the pin's location
        let pinLocation = CLLocation(latitude: pin.coordinate.latitude, longitude: pin.coordinate.longitude)
        
        // Calculate the distance in meters
        let distanceInMeters = currentLocation.distance(from: pinLocation)
        
        // Convert distance to feet
        let distanceInFeet = distanceInMeters * 3.28084
        
        // Update the UI or print the distance
        print("Distance to pin: \(distanceInFeet) feet")
        
        // Example: Update a label with the distance
        distanceButton.setTitle(String(format: "Distance to LV: %.2f Feet", distanceInFeet), for: .normal)
    }
    
    @objc func handleTripleTap(_ gesture: UITapGestureRecognizer) {
        let storybaord = UIStoryboard(name: "Main", bundle: nil)
        let MapStoryboard = storybaord.instantiateViewController(withIdentifier: "MapView") as! MapViewController
        MapStoryboard.loadViewIfNeeded()
        
        
        self.present(MapStoryboard, animated: true, completion: nil)
    }
}


