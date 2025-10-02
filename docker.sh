
cd client && npm run prod
cd ../docker/client
docker build --no-cache -t smartsolar-client:latest .
docker image save smartsolar-client -o ../smartsolar-client.img

cd ../../api && npm run build;
cp package.json ../docker/api/app/package.json
cp tsconfig.json ../docker/api/app/tsconfig.json
cd ../docker/api
docker build --no-cache -t smartsolar-api:latest .
docker image save smartsolar-api -o ../smartsolar-api.img
