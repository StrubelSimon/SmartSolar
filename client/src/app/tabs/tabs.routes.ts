import { Routes } from '@angular/router';
import { TabsPage } from './tabs.page';

export const routes: Routes = [
  {
    path: 'tabs',
    component: TabsPage,
    children: [
      {
        path: 'pv',
        loadComponent: () =>
          import('./pv/pv.page').then((m) => m.PVPage),
      },
      {
        path: 'battery',
        loadComponent: () =>
          import('./battery/battery.page').then((m) => m.BatteryPage),
      },
      {
        path: 'load',
        loadComponent: () =>
          import('./load/load.page').then((m) => m.LoadPage),
      },
      {
        path: '',
        redirectTo: '/tabs/battery',
        pathMatch: 'full',
      },
    ],
  },
  {
    path: '',
    redirectTo: '/tabs/battery',
    pathMatch: 'full',
  },
];
