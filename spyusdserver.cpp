/*!
    \file matching_engine.cpp
    \brief Matching engine example
    \author Ivan Shynkarenka
    \date 16.08.2017
    \copyright MIT License
*/
#include <poll.h>
#include <json/json.h>
#include "trader/matching/market_manager.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "system/stream.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <regex>
#include <string>

using namespace CppTrader::Matching;

class MyMarketHandler : public MarketHandler
{
void levelsend(const Level* node, std::string c)
{
	int pid;
	char* call = (char*)"/tmp/callbackpipe";
	mkfifo(call, 0666);
	pid = open(call, O_WRONLY);
	std::string type;
 	if(node->IsBid()){type="+";}
	else{type="-";}	
	std::string s = "["+std::to_string(node->Price)+","+std::to_string(node->TotalVolume)+","+type+"]"+c;
	write(pid, s.c_str(), s.length());
}
void pipesend(const char* message);
std::string traverse(const LevelNode* node, std::string ret)
{return "";
/*	if(node==NULL)
	{
	//ret = ret+"["+std::to_string(node->Price)+","+std::to_string(node->TotalVolume)+"]";
	//	return traverse(node->left,ret) + traverse(node->right,ret);
	return ret;
	}	
	else{
	std::string rv;
	rv = traverse(node->left, ret+=);
	if(!rv.empty())
	{return rv};
	ret = 
	}
	//return ret;*/
}
protected:
    void onAddSymbol(const Symbol& symbol) override
    { std::cout << "Add symbol: " << symbol << std::endl; }
    void onDeleteSymbol(const Symbol& symbol) override
    { std::cout << "Delete symbol: " << symbol << std::endl; }

    void onAddOrderBook(const OrderBook& order_book) override
    { std::cout << "Add order book: " << order_book << std::endl; }
    void onUpdateOrderBook(const OrderBook& order_book, bool top) override
    { std::cout << "Update order book: " << order_book << (top ? " - Top of the book!" : "") << std::endl;
      //pipesend("{\"UpdateBook\":\""+order_book.bids()+order_book.asks()+"\"}");	    
      //pipesend(order_book.symbol().Name);
    /*  int ps; char* call = (char*)"/tmp/callbackpipe";
      mkfifo(call, 0666);
     ps = open(call, O_WRONLY);std::string s = traverse(order_book.bids().root(), "")+"bids"+traverse(order_book.asks().root(), "")+"asks~";
     write(ps, s.c_str(), s.length());*/
    // close(ps);
     // unlink(call); 
    }
    void onDeleteOrderBook(const OrderBook& order_book) override
    { std::cout << "Delete order book: " << order_book << std::endl; }

    void onAddLevel(const OrderBook& order_book, const Level& level, bool top) override
    { std::cout << "Add level: " << level << (top ? " - Top of the book!" : "") << std::endl;levelsend(&level, "a"); }
    void onUpdateLevel(const OrderBook& order_book, const Level& level, bool top) override
    { std::cout << "Update level: " << level << (top ? " - Top of the book!" : "") << std::endl;levelsend(&level, "u"); }
    void onDeleteLevel(const OrderBook& order_book, const Level& level, bool top) override
    { std::cout << "Delete level: " << level << (top ? " - Top of the book!" : "") << std::endl;levelsend(&level, "d"); }

    void onAddOrder(const Order& order) override
    { std::cout << "Add order: " << order << std::endl; }
    void onUpdateOrder(const Order& order) override
    { std::cout << "Update order: " << order << std::endl; }
    void onDeleteOrder(const Order& order) override
    { std::cout << "Delete order: " << order << std::endl;
   	//pipesend("{\"DeleteOrder\":\""+(std::string)order.Quantity+"\"}");
	/*int ps; char* call = (char*)"/tmp/callbackpipe";
	mkfifo(call, 0666);
	ps = open(call, O_WRONLY);std::string s = std::to_string(order.Id);size_t t = s.length();
	write(ps,s.c_str(),t);
//	close(ps); unlink(call);*/
    }

    void onExecuteOrder(const Order& order, uint64_t price, uint64_t quantity) override
    { std::cout << "Execute order: " << order << " with price " << price << " and quantity " << quantity << std::endl; }
};
void pipesend(const char* message)
{
	int ps;
	char *callbackpipe = (char*)"/tmp/callbackpipe";
	mkfifo(callbackpipe, 0666);
	ps = open(callbackpipe, O_WRONLY);
	write(ps, message, strlen(message));
	close(ps);
	unlink(callbackpipe);
}
void AddSymbol(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add symbol (\\d+) (.+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint32_t id = std::stoi(match[1]);

        char name[8];
        std::string sname = match[2];
        std::memcpy(name, sname.data(), std::min(sname.size(), sizeof(name)));

        Symbol symbol(id, name);

        ErrorCode result = market.AddSymbol(symbol);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add symbol' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add symbol' command: " << command << std::endl;
}

void DeleteSymbol(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^delete symbol (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint32_t id = std::stoi(match[1]);

        ErrorCode result = market.DeleteSymbol(id);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'delete symbol' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'delete symbol' command: " << command << std::endl;
}

void AddOrderBook(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add book (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint32_t id = std::stoi(match[1]);

        char name[8];
        std::memset(name, 0, sizeof(name));

        Symbol symbol(id, name);

        ErrorCode result = market.AddOrderBook(symbol);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add book' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add book' command: " << command << std::endl;
}

void DeleteOrderBook(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^delete book (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint32_t id = std::stoi(match[1]);

        ErrorCode result = market.DeleteOrderBook(id);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'delete book' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'delete book' command: " << command << std::endl;
}

void AddMarketOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add market (buy|sell) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t quantity = std::stoi(match[4]);

        Order order;
        if (match[1] == "buy")
            order = Order::BuyMarket(id, symbol_id, quantity);
        else if (match[1] == "sell")
            order = Order::SellMarket(id, symbol_id, quantity);
        else
        {
            std::cerr << "Invalid market order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add market' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add market' command: " << command << std::endl;
}

void AddSlippageMarketOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add slippage market (buy|sell) (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t quantity = std::stoi(match[4]);
        uint64_t slippage = std::stoi(match[5]);

        Order order;
        if (match[1] == "buy")
            order = Order::BuyMarket(id, symbol_id, quantity, slippage);
        else if (match[1] == "sell")
            order = Order::SellMarket(id, symbol_id, quantity, slippage);
        else
        {
            std::cerr << "Invalid market order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add slippage market' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add slippage market' command: " << command << std::endl;
}

void AddLimitOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add limit (buy|sell) (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t price = std::stoi(match[4]);
        uint64_t quantity = std::stoi(match[5]);

        Order order;
        if (match[1] == "buy")
            order = Order::BuyLimit(id, symbol_id, price, quantity);
        else if (match[1] == "sell")
            order = Order::SellLimit(id, symbol_id, price, quantity);
        else
        {
            std::cerr << "Invalid limit order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add limit' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add limit' command: " << command << std::endl;
}

void AddIOCLimitOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add ioc limit (buy|sell) (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t price = std::stoi(match[4]);
        uint64_t quantity = std::stoi(match[5]);

        Order order;
        if (match[1] == "buy")
            order = Order::BuyLimit(id, symbol_id, price, quantity, OrderTimeInForce::IOC);
        else if (match[1] == "sell")
            order = Order::SellLimit(id, symbol_id, price, quantity, OrderTimeInForce::IOC);
        else
        {
            std::cerr << "Invalid limit order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add ioc limit' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add ioc limit' command: " << command << std::endl;
}

void AddFOKLimitOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add fok limit (buy|sell) (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t price = std::stoi(match[4]);
        uint64_t quantity = std::stoi(match[5]);

        Order order;
        if (match[1] == "buy")
            order = Order::BuyLimit(id, symbol_id, price, quantity, OrderTimeInForce::FOK);
        else if (match[1] == "sell")
            order = Order::SellLimit(id, symbol_id, price, quantity, OrderTimeInForce::FOK);
        else
        {
            std::cerr << "Invalid limit order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add fok limit' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add fok limit' command: " << command << std::endl;
}

void AddAONLimitOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add aon limit (buy|sell) (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t price = std::stoi(match[4]);
        uint64_t quantity = std::stoi(match[5]);

        Order order;
        if (match[1] == "buy")
            order = Order::BuyLimit(id, symbol_id, price, quantity, OrderTimeInForce::AON);
        else if (match[1] == "sell")
            order = Order::SellLimit(id, symbol_id, price, quantity, OrderTimeInForce::AON);
        else
        {
            std::cerr << "Invalid limit order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add aon limit' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add aon limit' command: " << command << std::endl;
}

void AddStopOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add stop (buy|sell) (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t stop_price = std::stoi(match[4]);
        uint64_t quantity = std::stoi(match[5]);

        Order order;
        if (match[1] == "buy")
            order = Order::BuyStop(id, symbol_id, stop_price, quantity);
        else if (match[1] == "sell")
            order = Order::SellStop(id, symbol_id, stop_price, quantity);
        else
        {
            std::cerr << "Invalid stop order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add stop' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add stop' command: " << command << std::endl;
}

void AddStopLimitOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add stop-limit (buy|sell) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t stop_price = std::stoi(match[4]);
        uint64_t price = std::stoi(match[5]);
        uint64_t quantity = std::stoi(match[6]);

        Order order;
        if (match[1] == "buy")
            order = Order::BuyStopLimit(id, symbol_id, stop_price, price, quantity);
        else if (match[1] == "sell")
            order = Order::SellStopLimit(id, symbol_id, stop_price, price, quantity);
        else
        {
            std::cerr << "Invalid stop-limit order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add stop-limit' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add stop-limit' command: " << command << std::endl;
}

void AddTrailingStopOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add trailing stop (buy|sell) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t stop_price = std::stoi(match[4]);
        uint64_t quantity = std::stoi(match[5]);
        int64_t trailing_distance = std::stoi(match[6]);
        int64_t trailing_step = std::stoi(match[7]);

        Order order;
        if (match[1] == "buy")
            order = Order::TrailingBuyStop(id, symbol_id, stop_price, quantity, trailing_distance, trailing_step);
        else if (match[1] == "sell")
            order = Order::TrailingSellStop(id, symbol_id, stop_price, quantity, trailing_distance, trailing_step);
        else
        {
            std::cerr << "Invalid stop order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add trailing stop' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add trailing stop' command: " << command << std::endl;
}

void AddTrailingStopLimitOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^add trailing stop-limit (buy|sell) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[2]);
        uint32_t symbol_id = std::stoi(match[3]);
        uint64_t stop_price = std::stoi(match[4]);
        uint64_t price = std::stoi(match[5]);
        uint64_t quantity = std::stoi(match[6]);
        int64_t trailing_distance = std::stoi(match[7]);
        int64_t trailing_step = std::stoi(match[8]);

        Order order;
        if (match[1] == "buy")
            order = Order::TrailingBuyStopLimit(id, symbol_id, stop_price, price, quantity, trailing_distance, trailing_step);
        else if (match[1] == "sell")
            order = Order::TrailingSellStopLimit(id, symbol_id, stop_price, price, quantity, trailing_distance, trailing_step);
        else
        {
            std::cerr << "Invalid stop-limit order side: " << match[1] << std::endl;
            return;
        }

        ErrorCode result = market.AddOrder(order);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'add trailing stop-limit' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'add trailing stop-limit' command: " << command << std::endl;
}

void ReduceOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^reduce order (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[1]);
        uint64_t quantity = std::stoi(match[2]);

        ErrorCode result = market.ReduceOrder(id, quantity);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'reduce order' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'reduce order' command: " << command << std::endl;
}

void ModifyOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^modify order (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[1]);
        uint64_t new_price = std::stoi(match[2]);
        uint64_t new_quantity = std::stoi(match[3]);

        ErrorCode result = market.ModifyOrder(id, new_price, new_quantity);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'modify order' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'modify order' command: " << command << std::endl;
}

void MitigateOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^mitigate order (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[1]);
        uint64_t new_price = std::stoi(match[2]);
        uint64_t new_quantity = std::stoi(match[3]);

        ErrorCode result = market.MitigateOrder(id, new_price, new_quantity);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'mitigate order' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'mitigate order' command: " << command << std::endl;
}

void ReplaceOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^replace order (\\d+) (\\d+) (\\d+) (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[1]);
        uint64_t new_id = std::stoi(match[2]);
        uint64_t new_price = std::stoi(match[3]);
        uint64_t new_quantity = std::stoi(match[4]);

        ErrorCode result = market.ReplaceOrder(id, new_id, new_price, new_quantity);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'replace order' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'replace order' command: " << command << std::endl;
}

void DeleteOrder(MarketManager& market, const std::string& command)
{
    static std::regex pattern("^delete order (\\d+)$");
    std::smatch match;

    if (std::regex_search(command, match, pattern))
    {
        uint64_t id = std::stoi(match[1]);

        ErrorCode result = market.DeleteOrder(id);
        if (result != ErrorCode::OK)
            std::cerr << "Failed 'delete order' command: " << result << std::endl;

        return;
    }

    std::cerr << "Invalid 'delete order' command: " << command << std::endl;
}

int main(int argc, char** argv)
{
    MyMarketHandler market_handler;
    MarketManager market(market_handler);

    
    // Perform text input
    std::string line;
    while(true){
	    while(true){
	/*key_t key = 6969;
    	int shmid = shmget(key, 1024, 0666|IPC_CREAT);
	if(shmid < 1) {continue;}
	char *str = (char*) shmat(shmid,(void*)0,0);	    
	//printf("data read: %s\n", str);
	//shmdt(str);
	//shmctl(shmid, IPC_RMID, NULL);
	if(str[0] == '{' || strlen(str) > 10){
		int size = strlen(str)+1;
		line.assign(str, size);
		break;
	}
	line.assign(str, strlen(str)+1);
	if(line.length() < 1) {continue;}
	std::cout << "-" << line << std::endl;
	shmdt(str);
	shmctl(shmid,IPC_RMID,NULL);*/
	struct pollfd pfds[1];
	pfds[0].fd = 0;
	pfds[0].events = POLLIN;
 	int rv;
	int fd;
	char * myfifo = (char *)"/tmp/myfifo";
	char buf[1024];
//	std::cout << "-" << std::endl;
	fd = open(myfifo, O_RDONLY);
	
	if((rv = poll(pfds,1,1))==-1) {perror("error"); continue;}
	if(fd<0){continue;}
	//else if (rv == 0){continue;}
	// {std::cout << "timeout" << std::endl;}
	else if (pfds[0].revents & POLLIN) {std::cout << "-" << std::endl;}
//	std::cout << fd << std::endl;
	read(fd, buf, 1024);
//	printf("received: %s \n", buf);
	//close(fd);
	//unlink(myfifo);
	std::string tempstr;
	tempstr.assign(buf, strlen(buf)+1);
	std::cout << tempstr << std::endl;
	int tempit = 0;
	std::string method, one, two, three, four, five, six, seven, eight, nine;
	//std::cout << tempstr[0] << std::endl;
	for(int it = 1;(unsigned)it<tempstr.size();it++)
	{
		if(tempstr[it]=='"') //opening first
		{
//			std::cout << tempstr[i] << std::endl;
			it+=1;
			while(tempstr[it]!='"') //closing first
			{it+=1; }
			it+=1;
			while(tempstr[it]!='"') //opening second
			{it+=1; }
			it+=1;
			while(tempstr[it]!='"') //run till closing second
			{
				if(tempit==0){method+=tempstr[it];}
				else if(tempit==1){one+=tempstr[it];}
				else if (tempit==2){two+=tempstr[it];}
				else if (tempit==3){three+=tempstr[it];}	
				else if (tempit==4){four+=tempstr[it];}
				else if (tempit==5){five+=tempstr[it];}
				else if (tempit==6){six+=tempstr[it];}
				else if (tempit==7){seven+=tempstr[it];}
				else if (tempit==8){eight+=tempstr[it];}
				else if (tempit==9){nine+=tempstr[it];}
				it+=1;
			}
			tempit+=1;
		}	
		//tempit += 1;
		//std::cout << tempit << std::endl;
	}
	std::cout <<one+two+three+four+five+six+seven+eight+nine<< std::endl;
	std::cout << method << std::endl;
	
	std::string contractid;
	if(six=="SPYUSD"){contractid="0";}
	else if(six=="SPYBTC"){contractid="1";}
//	else {continue;}

	if(method=="Post"&&five=="limit"){line = "add limit "+one+" "+two+" "+contractid+" "+three+" "+four;}
	else if(method=="Post"&&five=="market"){line = "add slippage market "+one+" "+two+" "+contractid+" "+three+" "+four;}
	else if(method=="Put"){line = "modify order "+one+" "+two+" "+three;}
	else if(method=="Delete"){line = "delete order "+one;}
	else if(method=="Admin"){line = one;}
	//line = method;
 	//line.assign(buf, strlen(buf)+1);
	//std::cout << "-" << std::endl;
	//if(fd <0){continue;}

	std::cout << line << std::endl;

	
	break;
	
	}
	    	
        if (line == "help")
        {
            std::cout << "Supported commands: " << std::endl;
            std::cout << "add symbol {Id} {Name} - Add a new symbol with {Id} and {Name}" << std::endl;
            std::cout << "delete symbol {Id} - Delete the symbol with {Id}" << std::endl;
            std::cout << "add book {Id} - Add a new order book for the symbol with {Id}" << std::endl;
            std::cout << "delete book {Id} - Delete the order book with {Id}" << std::endl;
            std::cout << "add market {Side} {Id} {SymbolId} {Quantity} - Add a new market order of {Type} (buy/sell) with {Id}, {SymbolId} and {Quantity}" << std::endl;
            std::cout << "add slippage market {Side} {Id} {SymbolId} {Quantity} {Slippage} - Add a new slippage market order of {Type} (buy/sell) with {Id}, {SymbolId}, {Quantity} and {Slippage}" << std::endl;
            std::cout << "add limit {Side} {Id} {SymbolId} {Price} {Quantity} - Add a new limit order of {Type} (buy/sell) with {Id}, {SymbolId}, {Price} and {Quantity}" << std::endl;
            std::cout << "add ioc limit {Side} {Id} {SymbolId} {Price} {Quantity} - Add a new 'Immediate-Or-Cancel' limit order of {Type} (buy/sell) with {Id}, {SymbolId}, {Price} and {Quantity}" << std::endl;
            std::cout << "add fok limit {Side} {Id} {SymbolId} {Price} {Quantity} - Add a new 'Fill-Or-Kill' limit order of {Type} (buy/sell) with {Id}, {SymbolId}, {Price} and {Quantity}" << std::endl;
            std::cout << "add aon limit {Side} {Id} {SymbolId} {Price} {Quantity} - Add a new 'All-Or-None' limit order of {Type} (buy/sell) with {Id}, {SymbolId}, {Price} and {Quantity}" << std::endl;
            std::cout << "add stop {Side} {Id} {SymbolId} {StopPrice} {Quantity} - Add a new stop order of {Type} (buy/sell) with {Id}, {SymbolId}, {StopPrice} and {Quantity}" << std::endl;
            std::cout << "add stop-limit {Side} {Id} {SymbolId} {StopPrice} {Price} {Quantity} - Add a new stop-limit order of {Type} (buy/sell) with {Id}, {SymbolId}, {StopPrice}, {Price} and {Quantity}" << std::endl;
            std::cout << "add trailing stop {Side} {Id} {SymbolId} {StopPrice} {Quantity} {TrailingDistance} {TrailingStep} - Add a new trailing stop order of {Type} (buy/sell) with {Id}, {SymbolId}, {StopPrice}, {Quantity}, {TrailingDistance} and {TrailingStep}" << std::endl;
            std::cout << "add trailing stop-limit {Side} {Id} {SymbolId} {StopPrice} {Price} {Quantity} {TrailingDistance} {TrailingStep} - Add a new trailing stop-limit order of {Type} (buy/sell) with {Id}, {SymbolId}, {StopPrice}, {Price}, {Quantity}, {TrailingDistance} and {TrailingStep}" << std::endl;
            std::cout << "reduce order {Id} {Quantity} - Reduce the order with {Id} by the given {Quantity}" << std::endl;
            std::cout << "modify order {Id} {NewPrice} {NewQuantity} - Modify the order with {Id} and set {NewPrice} and {NewQuantity}" << std::endl;
            std::cout << "mitigate order {Id} {NewPrice} {NewQuantity} - Mitigate the order with {Id} and set {NewPrice} and {NewQuantity}" << std::endl;
            std::cout << "replace order {Id} {NewId} {NewPrice} {NewQuantity} - Replace the order with {Id} and set {NewId}, {NewPrice} and {NewQuantity}" << std::endl;
            std::cout << "delete order {Id} - Delete the order with {Id}" << std::endl;
            std::cout << "exit/quit - Exit the program" << std::endl;
        }
        else if ((line == "exit") || (line == "quit"))
            break;
        else if ((line.find("#") == 0) || (line == ""))
            continue;
        else if (line == "enable matching")
            market.EnableMatching();
        else if (line == "disable matching")
            market.DisableMatching();
        else if (line.find("add symbol") != std::string::npos)
            AddSymbol(market, line);
        else if (line.find("delete symbol") != std::string::npos)
            DeleteSymbol(market, line);
        else if (line.find("add book") != std::string::npos)
            AddOrderBook(market, line);
        else if (line.find("delete book") != std::string::npos)
            DeleteOrderBook(market, line);
        else if (line.find("add market") != std::string::npos)
            AddMarketOrder(market, line);
        else if (line.find("add slippage market") != std::string::npos)
            AddSlippageMarketOrder(market, line);
        else if (line.find("add limit") != std::string::npos)
            AddLimitOrder(market, line);
        else if (line.find("add ioc limit") != std::string::npos)
            AddIOCLimitOrder(market, line);
        else if (line.find("add fok limit") != std::string::npos)
            AddFOKLimitOrder(market, line);
        else if (line.find("add aon limit") != std::string::npos)
            AddAONLimitOrder(market, line);
        else if (line.find("add stop-limit") != std::string::npos)
            AddStopLimitOrder(market, line);
        else if (line.find("add stop") != std::string::npos)
            AddStopOrder(market, line);
        else if (line.find("add trailing stop-limit") != std::string::npos)
            AddTrailingStopLimitOrder(market, line);
        else if (line.find("add trailing stop") != std::string::npos)
            AddTrailingStopOrder(market, line);
        else if (line.find("reduce order") != std::string::npos)
            ReduceOrder(market, line);
        else if (line.find("modify order") != std::string::npos)
            ModifyOrder(market, line);
        else if (line.find("mitigate order") != std::string::npos)
            MitigateOrder(market, line);
        else if (line.find("replace order") != std::string::npos)
            ReplaceOrder(market, line);
        else if (line.find("delete order") != std::string::npos)
            DeleteOrder(market, line);
        else
            std::cerr << "Unknown command: "  << line << std::endl;
    }

    return 0;
}
