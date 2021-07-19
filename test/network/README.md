Buildroot Networking Test

# Current Status
This test is not fully developed. For now, network.yaml is simply an
interactive 2-job workload.

# Overview
Jobs are expected to run simultaneously with a network between them. In this
test, we use a simple "ping/pong" to ensure they can contact eachother. We also
check internet access by accessing www.google.com.
