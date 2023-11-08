#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>

int main() {
  using namespace httplib;
  using namespace nlohmann;

  Server svr;

  enum State{ 
    INITIAL,
    TEST_HGET_HGET,
    TEST_HGET_END,
  } state = INITIAL;

  svr.Post("/path", [&state](const auto& req, auto &res){
    std::cout << "Received POST from client." << std::endl;
    std::cout << "Request headers are: " << std::endl;
    for(auto header : req.headers) {
      std::cout << header.first << " : " << header.second << std::endl;
    }
    std::cout << "Request body is:" << std::endl << req.body << std::endl;
    
    std::string response;
    json p2; json cmd;
    
    switch (state) {
      case INITIAL:
	std::cout << "State is INITIAL" << std::endl;	
	cmd.push_back(json({"HGET","http://localhost:9988/test_hget","urn:oma:mo:oma-fumo:1.0/apps/DownloadAndUpdate/PkgURL"}));
	p2["CMD"] = cmd;
	response = p2.dump();
	state = TEST_HGET_HGET;
	break;
      case TEST_HGET_END:
	std::cout << "State is TEST_HGET" << std::endl; 
	cmd.push_back(json({"END"}));
	p2["CMD"] = cmd;
	response = p2.dump();
	std::cout << "FINISHED" << std::endl;
	exit(0);
	break;
      default:
	cmd.push_back(json({"END"}));
	p2["CMD"] = cmd;
	response = p2.dump();
	std::cerr << "UNEXPECTED protocol package received during HGET test (was expecting HGET http connection)" << std::endl;
	exit(1);
	break;
    }


    res.set_content(response, "application/vnd.oma.dm.request+json");

    std::cout << "Responding to client." << std::endl;
    std::cout << "Response headers are: " << std::endl;
    for(auto header : res.headers) {
      std::cout << header.first << " : " << header.second << std::endl;
    }

    std::cout << "Response body is:" << std::endl << res.body << std::endl;
  });


  svr.Get("/test_hget", [&state](const auto &req, auto &res) {

    if(state != TEST_HGET_HGET) {
      std::cerr << "UNEXPECTED HGET http connection received" << std::endl;
      exit(1);
    }
    std::cout << "Received GET from client." << std::endl;
    std::cout << "Request headers are: " << std::endl;
    for(auto header : req.headers) {
      std::cout << header.first << " : " << header.second << std::endl;
    }
    std::cout << "Request body is:" << std::endl << req.body << std::endl;

    json jres;
    jres["MOData"]["fumo"]["DownloadAndUpdate"]["PkgURL"] = "COFFEEBABE";

    std::string response = jres.dump();
    res.set_content(response, "application/dmmo+json");

    std::cout << "Response body is:" << std::endl << res.body << std::endl;

  });
  svr.listen("localhost", 9988);

  return 0;
}
