import { server } from './app'

const start = async () => {

  let expressPort = 3000;

  if (process.env.PORT) {
    expressPort = +process.env.PORT
  }

  server.listen(expressPort, function () {
    console.log(`Listening on port ${expressPort}`);
  });

}

start()