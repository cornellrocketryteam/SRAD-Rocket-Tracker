//
//  MapViewController.swift
//  Rocket_Tracker
//
//  Created by Ronit Totlani on 3/24/25.
//

import UIKit
import Foundation
import CoreBluetooth
import MapKit
import CoreLocation

class MapViewController: UIViewController {
    @IBAction func backButton(_ sender: UIButton) {
        dismiss(animated: true, completion: nil)
    }
    
    
    @IBOutlet weak var bigMapView: MKMapView!
    override func viewDidLoad() {
        let launchVehicle = CLLocationCoordinate2D(latitude: latcurr, longitude: longcurr)
        let span = MKCoordinateSpan(latitudeDelta: 0.1, longitudeDelta: 0.1)
        let region = MKCoordinateRegion(center: launchVehicle, span: span)
        bigMapView.setRegion(region, animated: true)
        if (firstRecived) {
            bigMapView.addAnnotation(pin)
        }
        super.viewDidLoad()
        
    }

}
