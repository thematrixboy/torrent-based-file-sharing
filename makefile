all: script tracker1 tracker2 peer1 peer2 peer3

script: initial.sh
	chmod 777 initial.sh
	./initial.sh

tracker1: tracker1/tracker1.o tracker1/trackersync1.o
	g++ -pthread -Wall tracker1/tracker1.o tracker1/trackersync1.o -o tracker1/tracker1 -lssl -lcrypto

tracker1/tracker1.o: tracker/tracker.cpp
	g++ -pthread -Wall -c -o tracker1/tracker1.o tracker/tracker.cpp -lssl -lcrypto

tracker1/trackersync1.o: tracker/trackersync.cpp
	g++ -pthread -Wall -c -o tracker1/trackersync1.o tracker/trackersync.cpp -lssl -lcrypto

tracker2: tracker2/tracker2.o tracker2/trackersync2.o
	g++ -pthread -Wall tracker2/tracker2.o tracker2/trackersync2.o -o tracker2/tracker2 -lssl -lcrypto

tracker2/tracker2.o: tracker/tracker.cpp
	g++ -pthread -Wall -c -o tracker2/tracker2.o tracker/tracker.cpp -lssl -lcrypto

tracker2/trackersync2.o: tracker/trackersync.cpp
	g++ -pthread -Wall -c -o tracker2/trackersync2.o tracker/trackersync.cpp -lssl -lcrypto

peer1: peer1/peer1.o peer1/peerserver1.o peer1/peerdownload1.o
	g++ -pthread -Wall peer1/peer1.o peer1/peerserver1.o peer1/peerdownload1.o -o peer1/peer1 -lssl -lcrypto

peer1/peer1.o: peer/peer.cpp
	g++ -pthread -Wall -c -o peer1/peer1.o peer/peer.cpp -lssl -lcrypto

peer1/peerserver1.o: peer/peerserver.cpp 
	g++ -pthread -Wall -c -o peer1/peerserver1.o peer/peerserver.cpp -lssl -lcrypto

peer1/peerdownload1.o: peer/peerdownload.cpp
	g++ -pthread -Wall -c -o peer1/peerdownload1.o peer/peerdownload.cpp -lssl -lcrypto

peer2: peer2/peer2.o peer2/peerserver2.o peer2/peerdownload2.o
	g++ -pthread -Wall peer2/peer2.o peer2/peerserver2.o peer2/peerdownload2.o -o peer2/peer2 -lssl -lcrypto

peer2/peer2.o: peer/peer.cpp
	g++ -pthread -Wall -c -o peer2/peer2.o peer/peer.cpp -lssl -lcrypto

peer2/peerserver2.o: peer/peerserver.cpp 
	g++ -pthread -Wall -c -o peer2/peerserver2.o peer/peerserver.cpp -lssl -lcrypto

peer2/peerdownload2.o: peer/peerdownload.cpp
	g++ -pthread -Wall -c -o peer2/peerdownload2.o peer/peerdownload.cpp -lssl -lcrypto

peer3: peer3/peer3.o peer3/peerserver3.o peer3/peerdownload3.o
	g++ -pthread -Wall peer3/peer3.o peer3/peerserver3.o peer3/peerdownload3.o -o peer3/peer3 -lssl -lcrypto

peer3/peer3.o: peer/peer.cpp
	g++ -pthread -Wall -c -o peer3/peer3.o peer/peer.cpp -lssl -lcrypto

peer3/peerserver3.o: peer/peerserver.cpp 
	g++ -pthread -Wall -c -o peer3/peerserver3.o peer/peerserver.cpp -lssl -lcrypto

peer3/peerdownload3.o: peer/peerdownload.cpp
	g++ -pthread -Wall -c -o peer3/peerdownload3.o peer/peerdownload.cpp -lssl -lcrypto