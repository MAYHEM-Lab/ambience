from ambictl import Network, NetworkType

# Digitalocean SFO2 private network
network(native=Network("DO-SFO2", NetworkType.UDP))

# Fatih's home xbee network
network(native=Network("xbee-home", NetworkType.XBee))

network(native=Network("unix-hosted", NetworkType.UnixDomain, hop_cost=500))
