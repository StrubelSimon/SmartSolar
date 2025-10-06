import { CommonModule } from '@angular/common';
import { Component, OnDestroy, OnInit } from '@angular/core';
import { IonHeader, IonToolbar, IonTitle, IonContent, IonList, IonItem, IonLabel, IonIcon } from '@ionic/angular/standalone';
import { Subscription } from 'rxjs';
import { SocketService } from 'src/services/socket-service';

@Component({
  selector: 'app-tab1',
  templateUrl: 'pv.page.html',
  styleUrls: ['pv.page.scss'],
  imports: [
    CommonModule,
    IonHeader,
    IonToolbar,
    IonTitle,
    IonContent,
    IonList,
    IonItem,
    IonLabel,
    IonIcon
  ],
})
export class PVPage implements OnDestroy {

  public data: any = {
    voltage: 0,
    current: 0,
    power: 0,
    status: 0
  }

  private mqttSubscription!: Subscription;

  constructor(private socketService: SocketService) { }

  ionViewWillEnter(): void {
    this.loadData();
  }

  loadData(): void {
    // Subscribe auf /modbus/battery
    this.mqttSubscription = this.socketService.subscribeTopic('modbus/pv').subscribe((data: string) => {
      this.data = JSON.parse(data);
    });
  }
  ngOnDestroy(): void {
    // Unsubscribe beim Verlassen der Komponente
    this.socketService.unsubscribeTopic('modbus/pv');
    if (this.mqttSubscription) {
      this.mqttSubscription.unsubscribe();
    }
  }

}
