#include <iostream>
#include <memory>
#include <filesystem>
#include <fstream>

#include "DataFormats/FEDRawDataCollection.h"
#include "Framework/EDProducer.h"
#include "Framework/Event.h"
#include "Framework/EventSetup.h"
#include "Framework/PluginFactory.h"

#include "DataFormats/PointsCloud.h"

#include "SYCLCore/Product.h"
#include "SYCLCore/ScopedContext.h"

#include "SYCLDataFormats/PointsCloudSYCL.h"

class CLUEOutputProducer : public edm::EDProducer {
public:
  explicit CLUEOutputProducer(edm::ProductRegistry& reg);

private:
  void produce(edm::Event& event, edm::EventSetup const& eventSetup) override;
  edm::EDGetTokenT<cms::sycltools::Product<PointsCloudSYCL>> token_device_clusters;
  edm::EDPutTokenT<cms::sycltools::Product<PluginWrapper<PointsCloud,CLUEOutputProducer>>> token_output_dir;
};

CLUEOutputProducer::CLUEOutputProducer(edm::ProductRegistry& reg)
    : token_device_clusters(reg.consumes<cms::sycltools::Product<PointsCloudSYCL>>()),
      token_output_dir(reg.produces<cms::sycltools::Product<PluginWrapper<PointsCloud,CLUEOutputProducer>>>()) {}

void CLUEOutputProducer::produce(edm::Event& event, edm::EventSetup const& eventSetup) {
  bool verboseResults = true;
  auto outDir = eventSetup.get<std::filesystem::path>();

  auto const& pcProduct = event.get(token_device_clusters);
  cms::sycltools::ScopedContextProduce ctx{pcProduct};
  auto const& device_clusters = ctx.get(pcProduct);

  auto stream = ctx.stream();

  PointsCloud results(device_clusters.n);
  stream.memcpy(results.x.data(), device_clusters.x.get(), device_clusters.n * sizeof(float));
  stream.memcpy(results.y.data(), device_clusters.y.get(), device_clusters.n * sizeof(float));
  stream.memcpy(results.layer.data(), device_clusters.layer.get(), device_clusters.n * sizeof(int));
  stream.memcpy(results.weight.data(), device_clusters.weight.get(), device_clusters.n * sizeof(float));
  stream.memcpy(results.rho.data(), device_clusters.rho.get(), device_clusters.n * sizeof(float));
  stream.memcpy(results.delta.data(), device_clusters.delta.get(), device_clusters.n * sizeof(float));
  stream.memcpy(results.nearestHigher.data(), device_clusters.nearestHigher.get(), device_clusters.n * sizeof(int));
  stream.memcpy(results.isSeed.data(), device_clusters.isSeed.get(), device_clusters.n * sizeof(int));
  stream.memcpy(results.clusterIndex.data(), device_clusters.clusterIndex.get(), device_clusters.n * sizeof(int)).wait();

  std::cout << "Data transfered back to host" << std::endl;

  if (verboseResults) {
    std::ofstream clueOut(outDir);

    clueOut << "index,x,y,layer,weight,rho,delta,nh,isSeed,clusterId\n";
    for (int i = 0; i < device_clusters.n; i++) {
      clueOut << i << "," << results.x[i] << "," << results.y[i] << "," << results.layer[i] << "," << results.weight[i]
              << "," << results.rho[i] << "," << (results.delta[i] > 999 ? 999 : results.delta[i]) << ","
              << results.nearestHigher[i] << "," << results.isSeed[i] << "," << results.clusterIndex[i] << "\n";
    }

    clueOut.close();

    std::cout << "Ouput was saved in " << outDir << std::endl;
  }
  
  ctx.emplace(event, token_output_dir, std::move(results));
}
DEFINE_FWK_MODULE(CLUEOutputProducer);
