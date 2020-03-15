#include <map>
#include "ClientInfo.h"

class ClientList {
    public:
    ClientList();
    void add(ClientInfo ci);
    ClientInfo get_by_id(int id);
    
    private:
    std::map<int, ClientInfo> list;
};
