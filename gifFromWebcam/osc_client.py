import argparse
import json
import random
import time

from pythonosc import udp_client


if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("--ip", default="127.0.0.1",
  # parser.add_argument("--ip", default="et51.local",
      help="The ip of the OSC server")
  parser.add_argument("--port", type=int, default=6161,
      help="The port the OSC server is listening on")
  args = parser.parse_args()

  client = udp_client.SimpleUDPClient(args.ip, args.port)


game_id = "5d55c9b1c950e"


for i in range(3):
	client_data = json.dumps({
		# 'camera': i%2,
		'camera': i,
		'game_id': game_id
	})
	client.send_message("/game/start", client_data)
	time.sleep(2)



# time.sleep(2)
client_data = json.dumps({
	'game_id': game_id
})
client.send_message("/game/end", client_data)