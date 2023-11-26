FROM python:3.11.6
WORKDIR /coap-server
COPY . /coap-server
RUN pip3 install -r requirements.txt

EXPOSE 5683
CMD ["python3", "server.py"]