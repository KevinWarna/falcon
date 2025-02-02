#include <iostream>
#include <string>
#include "globals.h"
#include "AESObject.h"
#include "Precompute.h"
#include "secondary.h"
#include "connect.h"
#include "NeuralNetConfig.h"
#include "NeuralNetwork.h"
#include "unitTests.h"

#include "CNNLayer.h"
#include "FCLayer.h"

int partyNum;
AESObject* aes_indep;
AESObject* aes_next;
AESObject* aes_prev;
Precompute PrecomputeObject;

void print_vec(const RSSVectorMyType& vec) {
	cout << '[';
	for (size_t i = 0; i < (vec.size() > 10 ? 10 : vec.size()); i++) {
		cout << " (" << ((int) vec[i].first) << ", " << ((int) vec[i].second) << ')';
	}
	if (vec.size() > 10) { cout << " ..."; }
	cout << " ]" << endl;
}


int main(int argc, char** argv)
{
/****************************** PREPROCESSING ******************************/ 
	parseInputs(argc, argv);
	string network, dataset, security;
	bool PRELOADING = false;

/****************************** SELECT NETWORK ******************************/ 
	//Network {SecureML, Sarda, MiniONN, LeNet, AlexNet, and VGG16}
	//Dataset {MNIST, CIFAR10, and ImageNet}
	//Security {Semi-honest or Malicious}
	if (argc == 9)
	{network = argv[6]; dataset = argv[7]; security = argv[8];}
	else
	{
		network = "LeNet";
		dataset = "MNIST";
		security = "Semi-honest";
	}

	// Create the network
	NeuralNetwork* net;
	{
		// Populate the config
		NeuralNetConfig config(NUM_ITERATIONS);
		selectNetwork(network, dataset, security, &config);
		config.checkNetwork();

		// Create it
		net = new NeuralNetwork(&config);
	}

/****************************** AES SETUP and SYNC ******************************/ 
	aes_indep = new AESObject(argv[3]);
	aes_next = new AESObject(argv[4]);
	aes_prev = new AESObject(argv[5]);

	initializeCommunication(argv[2], partyNum);
	synchronize(2000000);

/****************************** RUN NETWORK/UNIT TESTS ******************************/ 
	//Run these if you want a preloaded network to be tested
	//assert(NUM_ITERATION == 1 and "check if readMiniBatch is false in test(net)")
	//First argument {SecureML, Sarda, MiniONN, or LeNet}
	// network += " preloaded"; PRELOADING = true;
	// preload_network(PRELOADING, network, net);

	#ifdef PRELOAD_NETWORK
	preload_network(true, network, dataset, net);
	#endif


	start_m();


	//Run inference (possibly with preloading a network)
	// network += " test";
	#ifdef PRELOAD_NETWORK
	test(true, network, dataset, net);
	#endif

	end_m(network);
	cout << "----------------------------------------------" << endl;  	
	cout << "Run details: " << NUM_OF_PARTIES << "PC (P" << partyNum 
		 << "), " << NUM_ITERATIONS << " iterations, batch size " << MINI_BATCH_SIZE << endl 
		 << "Running " << security << " " << network << " on " << dataset << " dataset" << endl;
	cout << "----------------------------------------------" << endl << endl;  

	printNetwork(net);

	// Finally, show the accuracy and junk
	if (network.compare("VGG16") == 0 && dataset.compare("MNIST") == 0) {
		net->collectMetrics(MINI_BATCH_SIZE, 33, 33, 3);
	} else if (dataset.compare("MNIST") == 0) {
		// I don't know the actual image sizes, just that the total should be 784...
		net->collectMetrics(MINI_BATCH_SIZE, 784, 1, 1);
	} else if (network.compare("AlexNet") == 0 && dataset.compare("CIFAR10") == 0) {
		net->collectMetrics(MINI_BATCH_SIZE, 33, 33, 3);
	} else if (network.compare("AlexNet") == 0 && dataset.compare("ImageNet") == 0) {
		net->collectMetrics(MINI_BATCH_SIZE, 56, 56, 3);
	} else if (network.compare("VGG16") == 0 && dataset.compare("CIFAR10") == 0) {
		net->collectMetrics(MINI_BATCH_SIZE, 32, 32, 3);
	} else if (network.compare("VGG16") == 0 && dataset.compare("ImageNet") == 0) {
		net->collectMetrics(MINI_BATCH_SIZE, 64, 64, 3);
	} else {
		cerr << "Encountered unknown neural network / dataset combination '" << network << "' / '" << dataset << '\'' << endl;
		return 1;
	}

	/****************************** CLEAN-UP ******************************/ 
	delete aes_indep;
	delete aes_next;
	delete aes_prev;
	delete net;
	deleteObjects();

	return 0;
}




