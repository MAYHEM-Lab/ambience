from math import sqrt
import tos.ae.agent
import lidlrt

def calc_values(sum, sq_sum, n):
    mean = sum/n
    variance = (sq_sum - (sum * sum)/n)/(n - 1)
    stddev = sqrt(variance)
    print(f"\tResults for {n} iterations:")
    print(f"\tMean: {mean}, Variance: {variance}, Stddev: {stddev}")
    
ip = "127.0.0.1"
port = 1234
agent = lidlrt.udp_client(tos.ae.agent.agent, (ip, port))
num_iterations = 100000
res = agent.start(param=num_iterations)
call_sum = res.total
call_sq_sum = res.median
return_sum = res.p90
return_sq_sum = res.p99

print("Time to call a service from another service:")
calc_values(call_sum, call_sq_sum, num_iterations)

print("Time to return from a service to another service:")
calc_values(return_sum, return_sq_sum, num_iterations)