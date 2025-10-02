import { ComponentFixture, TestBed } from '@angular/core/testing';

import { PVPage } from './pv.page';

describe('PVPage', () => {
  let component: PVPage;
  let fixture: ComponentFixture<PVPage>;

  beforeEach(async () => {
    fixture = TestBed.createComponent(PVPage);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
