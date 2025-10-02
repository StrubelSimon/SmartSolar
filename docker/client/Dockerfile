FROM nginx:stable
RUN apt-get update -y
RUN apt-get install nano -y

ENV NODE_ENV=production
# NGINX CONFIG
COPY nginx.conf /etc/nginx/nginx.conf
# CLIENT
COPY www /data/www

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]