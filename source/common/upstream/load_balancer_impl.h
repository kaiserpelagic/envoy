#pragma once

#include "envoy/runtime/runtime.h"
#include "envoy/upstream/load_balancer.h"
#include "envoy/upstream/upstream.h"

namespace Upstream {

/**
 * Base class for all LB implementations.
 */
class LoadBalancerBase {
protected:
  LoadBalancerBase(const HostSet& host_set, ClusterStats& stats, Runtime::Loader& runtime)
      : stats_(stats), runtime_(runtime), host_set_(host_set) {}

  /**
   * Pick the host list to use (healthy or all depending on how many in the set are not healthy).
   */
  const std::vector<HostPtr>& hostsToUse();

  ClusterStats& stats_;
  Runtime::Loader& runtime_;

private:
  const HostSet& host_set_;
};

/**
 * Implementation of LoadBalancer that performs RR selection across the hosts in the cluster.
 */
class RoundRobinLoadBalancer : public LoadBalancer, LoadBalancerBase {
public:
  RoundRobinLoadBalancer(const HostSet& host_set, ClusterStats& stats, Runtime::Loader& runtime)
      : LoadBalancerBase(host_set, stats, runtime) {}

  // Upstream::LoadBalancer
  ConstHostPtr chooseHost() override;

private:
  size_t rr_index_{};
};

/**
 * Weighted Least Request load balancer.
 *
 * In a normal setup when all hosts have the same weight of 1 it randomly picks up two healthy hosts
 * and compares number of active requests.
 * Technique is based on http://www.eecs.harvard.edu/~michaelm/postscripts/mythesis.pdf
 *
 * When any of the hosts have non 1 weight, apply random weighted balancing.
 * Randomly pickup the host and send 'weight' number of requests to it.
 * This technique is acceptable for load testing but
 * will not work well in situations where requests take a long time.
 * In that case a different algorithm using a full scan will be required.
 */
class LeastRequestLoadBalancer : public LoadBalancer, LoadBalancerBase {
public:
  LeastRequestLoadBalancer(const HostSet& host_set, ClusterStats& stats, Runtime::Loader& runtime,
                           Runtime::RandomGenerator& random);

  // Upstream::LoadBalancer
  ConstHostPtr chooseHost() override;

private:
  Runtime::RandomGenerator& random_;
  HostPtr last_host_;
  uint32_t hits_left_{};
};

/**
 * Random load balancer that picks a random host out of all hosts.
 */
class RandomLoadBalancer : public LoadBalancer, LoadBalancerBase {
public:
  RandomLoadBalancer(const HostSet& host_set, ClusterStats& stats, Runtime::Loader& runtime,
                     Runtime::RandomGenerator& random)
      : LoadBalancerBase(host_set, stats, runtime), random_(random) {}

  // Upstream::LoadBalancer
  ConstHostPtr chooseHost() override;

private:
  Runtime::RandomGenerator& random_;
};

} // Upstream
