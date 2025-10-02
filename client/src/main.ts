import { bootstrapApplication } from '@angular/platform-browser';
import { RouteReuseStrategy, provideRouter, withPreloading, PreloadAllModules } from '@angular/router';
import { IonicRouteStrategy, provideIonicAngular } from '@ionic/angular/standalone';

import { routes } from './app/app.routes';
import { AppComponent } from './app/app.component';
import { isDevMode } from '@angular/core';
import { provideServiceWorker } from '@angular/service-worker';
// import { MqttService, MqttServiceConfig } from 'ngx-mqtt';

// const MQTT_SERVICE_CONFIG: any = {
//   hostname: '192.168.178.200',
//   port: 8083,
//   protocol: 'ws',
//   username: 'admin',
//   password: '12345678',
//   clientId: 'angular-client-' + Math.random().toString(16).substr(2, 8),
// };

bootstrapApplication(AppComponent, {
  providers: [
    { provide: RouteReuseStrategy, useClass: IonicRouteStrategy },
    provideIonicAngular(),
    provideRouter(routes, withPreloading(PreloadAllModules)), provideServiceWorker('ngsw-worker.js', {
            enabled: !isDevMode(),
            registrationStrategy: 'registerWhenStable:30000'
          }),
  ],
});
