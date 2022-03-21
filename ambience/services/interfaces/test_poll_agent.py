import tos.ae.agent
import lidlrt

ip = "127.0.0.1"
port = 1234
agent = lidlrt.udp_client(tos.ae.agent.agent, (ip, port))
res = agent.start(param=100)
print(res)