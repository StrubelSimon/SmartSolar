import { Injectable } from '@angular/core';
import { io, Socket } from 'socket.io-client';
import { Observable } from 'rxjs';
import { environment } from "src/environments/environment";

@Injectable({
  providedIn: 'root',
})
export class SocketService {
  private socket: Socket;

  constructor() {
    this.socket = io(environment.socket);
  }

  subscribeTopic(topic: string): Observable<any> {
    this.socket.emit('subscribe', topic);
    return new Observable(observer => {
      this.socket.on('mqtt', (data) => {
        if (data.topic === topic) {
          observer.next(data.message);
        }
      });
    });
  }

  unsubscribeTopic(topic: string) {
    this.socket.emit('unsubscribe', topic);
  }
}
