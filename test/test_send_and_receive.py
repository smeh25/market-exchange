import zmq
import json
import time
import random
import threading

class ExchangeTester:
    def __init__(self, in_port="5555", out_port="5556"):
        self.context = zmq.Context()
        
        # Setup Senders (PUSH to 5555)
        self.sender = self.context.socket(zmq.PUSH)
        self.sender.set(zmq.SNDHWM, 100000)
        self.sender.connect(f"tcp://127.0.0.1:{in_port}")
        
        # Setup Receiver (PULL from 5556)
        self.receiver = self.context.socket(zmq.PULL)
        self.receiver.set(zmq.RCVHWM, 100000)
        self.receiver.bind(f"tcp://*:{out_port}")
        
        self.running = False
        self.response_count = 0
        self.drain_thread = None
        
        print(f"--- ExchangeTester Initialized (Idle) ---")



    def start_listening(self):
        """Explicitly starts the background drain thread."""
        if self.drain_thread is not None and self.drain_thread.is_alive():
            print("[WARN] Drain thread is already running.")
            return

        self.running = True
        self.drain_thread = threading.Thread(target=self._continuous_drain, daemon=True)
        self.drain_thread.start()
        print("[INFO] Background drain thread started.")
        time.sleep(0.5) # Small buffer for thread startup



    def _continuous_drain(self):
        """Background thread loop to pull messages constantly."""
        self.ack_count = 0
        self.rej_count = 0

        while self.running:
            if self.receiver.poll(100):
                try:
                    resp = self.receiver.recv_json()
                    
                    # Extract the message type from the header
                    msg_type = resp.get('header', {}).get('type')

                    if msg_type == 100:  # MsgType::Ack
                        self.ack_count += 1
                        self.response_count += 1
                        # print(f"[RECV ACK] ClientID: {resp['body'].get('client_order_id')} | Total Acks: {self.ack_count}")
                    
                    elif msg_type == 101:  # MsgType::Reject
                        self.rej_count += 1
                        self.response_count += 1
                        # reason = resp['body'].get('info', {}).get('reason', 'Unknown')
                        # print(f"[RECV REJECT] Reason: {reason} | Total Rejects: {self.rej_count}")
                    
                    elif msg_type == 102:  # MsgType::Fill
                        # print(f"[RECV FILL] OrderID: {resp['body'].get('order_id')} filled.")
                        return
                    
                    # Optional: Print the full raw JSON for deep debugging
                    # print(json.dumps(resp, indent=4))

                except Exception as e:
                    print(f"[DRAIN ERROR] {e}")

    def get_stats(self):
        """Returns the current message counts."""
        return {
            "total": self.response_count,
            "acks": self.ack_count,
            "rejects": self.rej_count
        }

    def send_valid(self, num=10):
        print(f"\n--- Sending {num} Valid Orders ---")
        symbols = ["AAPL", "TSLA", "GOOG", "MSFT"]
        for i in range(num):
            client_id = 20000 + i
            order = {
                "header": {"version": 1, "type": 1, "seq": i, "client_id": 55},
                "body": {
                    "client_order_id": client_id,
                    "symbol": random.choice(symbols),
                    "side": random.choice([1, 2]),
                    "ord_type": 2,
                    "qty": 100,
                    "limit_price": 15000
                }
            }
            self.sender.send_json(order)
        print(f"[DONE] Sent {num} valid orders.")



    def send_invalid(self, num=10):
        print(f"\n--- Sending {num} Malformed Messages ---")
        for i in range(num):
            bad_order = {"header": {"type": 1}, "body": {"garbage": True}}
            self.sender.send_json(bad_order)
        print(f"[DONE] Sent {num} malformed messages.")



    def stop(self):
        """Gracefully stops the background thread and cleans up ZMQ."""
        print(f"\n[INFO] Stopping tester. Final Response Count: {self.response_count}")
        self.running = False
        if self.drain_thread:
            self.drain_thread.join(timeout=2)
        
        self.sender.close()
        self.receiver.close()
        self.context.term()
        print("[INFO] Tester stopped and context terminated.")



if __name__ == "__main__":
    tester = ExchangeTester()
    
    # 1. Start the background consumer
    tester.start_listening()
    
    # 2. Run high-volume tests
    tester.send_valid(num=10)
    tester.send_invalid(num=10)
    
    # 3. Wait for workers to finish processing and sending Acks
    print("Waiting for final responses...")
    time.sleep(2)
    
    # 4. Cleanup
    tester.stop()

    stats = tester.get_stats()

    # Single line for ACKs
    print(f"Total Acknowledgements: {stats.get('acks')}")

    # Full summary line
    print(f"Final Report -> ACKs: {stats.get('acks')} | Rejects: {stats.get('rejects')} | Total: {stats.get('total')}")