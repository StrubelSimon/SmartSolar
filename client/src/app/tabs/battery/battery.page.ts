import { CommonModule } from '@angular/common';
import { Component, OnDestroy, OnInit } from '@angular/core';
import {
  IonHeader,
  IonToolbar,
  IonTitle,
  IonContent,
  IonList,
  IonItem,
  IonLabel,
  IonIcon
} from '@ionic/angular/standalone';

import { Subscription } from 'rxjs';
import { SocketService } from 'src/services/socket-service';

@Component({
  selector: 'app-tab2',
  templateUrl: 'battery.page.html',
  styleUrls: ['battery.page.scss'],
  imports: [
    CommonModule,
    IonHeader,
    IonToolbar,
    IonTitle,
    IonContent,
    IonList,
    IonItem,
    IonLabel,
    IonIcon],
})
export class BatteryPage implements OnDestroy {

  public data: any = {
    voltage: 0,
    current: 0,
    percentage: 0,
    charging: 0,
    voltageMin: 0,
    voltageMax: 0,
    temperature: 0,
    health: 0
  }
  private mqttSubscription!: Subscription;

  constructor(private socketService: SocketService) { }


  ionViewWillEnter(): void {
    this.loadData();
  }

  loadData(): void {
    // Subscribe auf /modbus/battery
    this.mqttSubscription = this.socketService.subscribeTopic('modbus/battery').subscribe((data: string) => {
      this.data = JSON.parse(data);
    });
  }
  ngOnDestroy(): void {
    // Unsubscribe beim Verlassen der Komponente
    this.socketService.unsubscribeTopic('modbus/battery');
    if (this.mqttSubscription) {
      this.mqttSubscription.unsubscribe();
    }
  }

}
