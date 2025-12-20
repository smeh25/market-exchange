import zmq
import time
import json

def run_test():
    # 1. Setup ZMQ context and PUSH socket
    context = zmq.Context()
    socket = context.socket(zmq.PUSH)
    
    # 2. Connect to your C++ program's port
    address = "tcp://localhost:5555"
    print(f"Connecting to {address}...")
    socket.connect(address)

    # 3. Define a few test messages
    messages = [
        {
            "header": {"version": 1, "type": 1, "seq": 1001, "client_id": 55},
            "body": {
                "client_order_id": 1,
                "symbol": "AAPL",
                "side": "BUY",
                "ord_type": "LIMIT",
                "qty": 10,
                "limit_price": 150
            }
        },
        {
            "header": {"version": 1, "type": 2, "seq": 1002, "client_id": 55},
            "body": {
                "symbol": "AAPL",
                "order_id": 500,
                "client_order_id": 1
            }
        },
        {"invalid": "data", "reason": "testing your error handling"}
    ]

    # 4. Send messages with a small delay
    for msg in messages:
        print(f"Sending: {msg['header']['type'] if 'header' in msg else 'Invalid Msg'}")
        socket.send_json(msg)
        time.sleep(1)

    print("Test complete.")

if __name__ == "__main__":
    run_test()