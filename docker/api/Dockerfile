FROM node:22.20.0-slim
RUN apt-get update -y
RUN apt-get install nano -y

ENV NODE_ENV=production

# API
RUN mkdir /app
WORKDIR /app

COPY app /app
RUN cd /app && npm install

EXPOSE 3000

CMD ["npm", "start"]