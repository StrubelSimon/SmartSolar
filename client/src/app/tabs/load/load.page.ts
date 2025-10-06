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
    this.mqttSubscription = this.socketService.subscribeTopic('modbus/load').subscribe((data: string) => {
      this.data = JSON.parse(data);
    });
  }

  ngOnDestroy(): void {
    // Unsubscribe beim Verlassen der Komponente
    this.socketService.unsubscribeTopic('modbus/load');
    if (this.mqttSubscription) {
      this.mqttSubscription.unsubscribe();
    }
  }

  toggleLoadState() {
    const payload = {
      topic: 'modbus/load/toggle',
      message: { command: 'turn_on_off', timestamp: Date.now() }
    };
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
