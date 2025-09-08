import asyncio
import argparse
import random

#Python script to test tcp server against concurrent connections and requests : format : 
#python3 clientScript.py --host 127.0.0.1 --port 9000--clients {n_clients} --duration {seconds}

async def client_task(client_id, host, port, duration,counter):
    try:
        reader , writer = await asyncio.open_connection(host,port)
        end_time = asyncio.get_event_loop().time() + duration
        while asyncio.get_event_loop().time() < end_time:
            if(random.randint(0,2) < 1):
                message = f"BUY {random.randint(100,1500)} {random.randint(1,500)}\n"
            else:
                message = f"SELL {random.randint(100,1500)} {random.randint(1,500)}\n"
            writer.write(message.encode())
            await writer.drain()
            try:
                data = await asyncio.wait_for(reader.read(1024),timeout = duration)
                print(f"Client {client_id} recieved {data.decode().strip()}")
                counter[0] += 1
            except asyncio.TimeoutError:
                pass
            #asyncio.sleep(random.uniform(0, 0.01))
        writer.close()
        await writer.wait_closed() 
    except Exception as e:
        print(f"Client {client_id} error: {e}")


async def main(host ,port, clients, duration):
    counter = [0] 
    tasks = [
        client_task(i, host, port, duration,counter)
        for i in range(clients)
    ]
    await asyncio.gather(*tasks)
    print(counter[0])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="TCP Load Tester")
    parser.add_argument("--host", default="0.0.0.0", help="Server host")
    parser.add_argument("--port", type=int, default=9000, help="Server port")
    parser.add_argument("--clients", type=int, default=100, help="Number of concurrent clients")
    parser.add_argument("--duration", type=int, default=5, help="Seconds each client stays connected")
    args = parser.parse_args()

    asyncio.run(main(args.host, args.port, args.clients, args.duration))


