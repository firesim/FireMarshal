import yaml
import sys

with open(sys.argv[1], 'r') as file:
    config = yaml.safe_load(file)

config["network"]["ethernets"]["ens33"]["addresses"] = ["172.16." + sys.argv[2] + "." + sys.argv[3] + "/24"]
config["network"]["ethernets"]["ens33"]["gateway4"] = ["172.16.1.1"]

with open(sys.argv[1], 'w') as file:
    yaml.dump(config, file)
