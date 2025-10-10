import { CommonModule } from '@angular/common';
import { Component, OnDestroy, OnInit } from '@angular/core';
import { IonHeader, IonToolbar, IonTitle, IonContent, IonList, IonItem, IonLabel, IonIcon, IonToggle } from '@ionic/angular/standalone';
import { Subscription } from 'rxjs';
import { SocketService } from 'src/services/socket-service';

@Component({
  selector: 'app-tab3',
  templateUrl: 'load.page.html',
  styleUrls: ['load.page.scss'],
  imports: [
    CommonModule,
    IonHeader,
    IonToolbar,
    IonTitle,
    IonContent,
    IonList,
    IonItem,
    IonLabel,
    IonIcon,
    IonToggle],
})
export class LoadPage implements OnDestroy {

  public loadDisabled: boolean = false;
  public oldLoadStatus: any;
  public data: any = {
    voltage: 0,
    current: 0,
    power: 0,
    status: 0,
    consumed_today: 0,
    consumed_month: 0,
    consumed_year: 0,
    consumed_total: 0
  }

  private mqttSubscription!: Subscription;

  constructor(private socketService: SocketService) { }

  ionViewWillEnter(): void {
    this.loadData();
  }

  loadData(): void {
    this.mqttSubscription = this.socketService.subscribeTopic('modbus/load').subscribe((data: string) => {
      this.data = JSON.parse(data);
      if (this.oldLoadStatus !== this.data.status) {
        this.loadDisabled = false;
      }

    });
  }

  ngOnDestroy(): void {
    // Unsubscribe beim Verlassen der Komponente
    this.socketService.unsubscribeTopic('modbus/load');
    if (this.mqttSubscription) {
      this.mqttSubscription.unsubscribe();
    }
  }

  toggleLoadState(status: any) {
    this.loadDisabled = true;
    this.oldLoadStatus = status;
    const payload = {
      topic: 'modbus/load/switch',
      message: { command: 'turn_on_off', timestamp: Date.now() }
    };
    if (status === 1) {
      payload.message.command = "off"
    } else {
      payload.message.command = "on"
    }
    this.socketService.sendCommand("publish", payload)
  }

  getLoadState(status: any) {
    if (status === 1) {
      return true;
    } else {
      return false;
    }
  }
}
