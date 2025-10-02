import { Component } from '@angular/core';
import { IonApp, IonRouterOutlet, Platform } from '@ionic/angular/standalone';
import { SwUpdateService } from 'src/services/sw-update.service';

@Component({
  selector: 'app-root',
  templateUrl: 'app.component.html',
  imports: [IonApp, IonRouterOutlet],
})
export class AppComponent {
  constructor(public platform: Platform, public swUpdateService: SwUpdateService) {

  }

  async ngOnInit() {
    await this.platform?.ready();
    this.swUpdateService.checkForUpdates();
  }

}
