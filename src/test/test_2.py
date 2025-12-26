import zmq
import json

context = zmq.Context()
# PUSH to the Exchange's PULL (In: 5555)
sender = context.socket(zmq.PUSH)
sender.connect("tcp://127.0.0.1:5555")

# PULL from the Exchange's PUSH (Out: 5556)
receiver = context.socket(zmq.PULL)
receiver.connect("tcp://127.0.0.1:5556")

# Following your ex namespace logic:
# Side: Buy = 1, Sell = 2
# MsgType: NewOrder = 1
# OrdType: Limit = 2
test_order = {
    "header": {
        "version": 1, 
        "type": 1,    # MsgType::NewOrder
        "seq": 1, 
        "client_id": 101
    },
    "body": {
        "client_order_id": 5001,
        "symbol": "AAPL",
        "side": 1,     # Side::Buy
        "ord_type": 2, # OrdType::Limit
        "qty": 100,
        "limit_price": 15050  # Using int64_t Price (scaled, e.g., cents)
    }
}

print(f"Sending Order ID {test_order['body']['client_order_id']}...")
sender.send_json(test_order)

# Listen for the Ack
if receiver.poll(3000): # Wait 3 seconds
    resp = receiver.recv_json()
    print("Received Response:", json.dumps(resp, indent=2))
else:
    print("No response from Exchange. Check if core is running.")