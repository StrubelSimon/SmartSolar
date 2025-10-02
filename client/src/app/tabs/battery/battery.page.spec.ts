import { ComponentFixture, TestBed } from '@angular/core/testing';

import { BatteryPage } from './battery.page';

describe('BatteryPage', () => {
  let component: BatteryPage;
  let fixture: ComponentFixture<BatteryPage>;

  beforeEach(async () => {
    fixture = TestBed.createComponent(BatteryPage);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
