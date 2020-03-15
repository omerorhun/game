#include "ClientList.h"
#include <iostream>

using namespace std;

void ClientList::add(ClientInfo ci) {
    list.insert(pair<int, ClientInfo>(ci.get_id, ci));
}

ClientInfo ClientList::get_by_id(int id) {
    ClientInfo client;
    client = list.find(id);
}