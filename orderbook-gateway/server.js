const {Kafka} = require('kafkajs');
const http = require('http');
const {WebSocketServer}= require('ws');

const uuidv4 = require('uuid').v4;

//Sever setuo
const server = http.createServer();
const webServer = new WebSocketServer({server});
const port = 8090;

//Server client structures
const connections = {};
const users = {};

//Kafka setup
const kafka = new Kafka({clientId: 'orderConsumer', brokers: ['localhost:9094']});
const consumer = kafka.consumer({groupId : 'gateway'});
const producer = kafka.producer();
const admin = kafka.admin();

// update user state 
const handleMessage = async (message, uuid) =>{
  const data = JSON.parse(message.toString());
  if(data.action == 'placeOrder'){
    users[uuid].orders += 1;
    connections[uuid].send(JSON.stringify(users[uuid]));
    await produceMessage(data);
  }
}

//Handle clients disconnecting
const handleClose = (uuid) => {
  delete connections[uuid];
  delete users[uuid];
}

//Handle new connections
webServer.on('connection',(connection) =>{
  const uuid = uuidv4();
  connections[uuid] = connection;

  users[uuid] = {
    orders: 0,
    type : 'Order Counter'
  }
  connection.on('message', message => handleMessage(message,uuid));
  connection.on('close', () => handleClose(uuid));
})

const produceMessage = async (message) => {
    try{
    const data = JSON.stringify(message);
        await producer.send({
            topic : 'incoming-orders',
            messages: [{value:data}]
        });
        console.log('message sent');
    }
    catch(err){
        console.error(err);
    }
}

async function run(){
  await admin.connect();
  await consumer.connect();
  await producer.connect();
  await admin.createTopics({topics : [{topic: 'incoming-orders'}]});
  await consumer.subscribe({ topic: 'orders', fromBeginning: true });

  await consumer.run({
    eachMessage: async({topic, partition, message}) => {
      try{
      webServer.clients.forEach(client =>{
        if(client.readyState == WebSocket.OPEN){
          client.send(message.value.toString());
        }
      });
    }
    catch(err){
      console.log(err);
      }
    }
  });
}

run().catch(console.error);


server.listen(port, () => {
  console.log(`Server is listening on ${port}`);
})

