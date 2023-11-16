#pragma once

#include <memory>
#include <set>

#include "MsgPackSocket.h"

namespace netLink {

//! Manages a group of Sockets
class SocketManager {
 public:
  // Event which is called if a TCP_SERVER accepts a new connection (if false is returned the connection will be closed
  // immediately)
  std::function<bool(SocketManager* manager, std::shared_ptr<Socket> serverSocket,
                     std::shared_ptr<Socket> clientSocket)>
      onConnectRequest;
  // Event which is called if a socket can or can not send more data (also called if nonblocking connect succeeded)
  std::function<void(SocketManager* manager, std::shared_ptr<Socket> socket, Socket::Status prev)> onStatusChange;
  // 收到 RawData 的回调函数
  std::function<void(SocketManager* manager, std::shared_ptr<Socket> socket)> onReceiveRaw;
  // 收到 MsgPack::Element 的回调函数
  std::function<void(SocketManager* manager, std::shared_ptr<Socket> socket, std::unique_ptr<MsgPack::Element> element)>
      onReceiveMsgPack;
  // 管理所有的 socket
  std::set<std::shared_ptr<Socket>> sockets;

  // 创建一个新的 Socket, 将其插入到 sockets 并返回
  std::shared_ptr<Socket> newSocket();
  // 创建一个新的 MsgPackSocket, 将其插入到 MsgPackSocket 并返回
  std::shared_ptr<Socket> newMsgPackSocket();

  /*Listens a periode time
   @param waitUpToSeconds Maximum time to wait for incoming data in seconds or negative values to wait indefinitely
   */
  void listen(double waitUpToSeconds = 0.0);
};

};  // namespace netLink
