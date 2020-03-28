/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#pragma once
#include "ipc-server.hpp"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#ifdef WIN32
#include "../source/windows/named-pipe.hpp"
#elif __APPLE__
#include "../source/apple/named-pipe.hpp"
#endif
#include <semaphore.h>

namespace ipc {
	class server;

	class server_instance {
		friend class server;

		public:
		server_instance();
#ifdef WIN32
		server_instance(server* owner, std::shared_ptr<os::windows::named_pipe> conn);
#elif __APPLE__
		server_instance(server* owner, std::shared_ptr<os::apple::named_pipe> conn);
#endif
        ~server_instance();
		
		bool is_alive();

		private: // Threading
		bool m_stopWorkers = false;
		std::thread m_worker_requests;
		std::thread m_worker_replies;

		std::string reader_sem_name = "semaphore-server-reader";
		std::string writer_sem_name = "semaphore-server-writer";
		sem_t *m_reader_sem;
		sem_t *m_writer_sem;

		void worker_req();
		void worker_rep();
		void read_callback_init(os::error ec, size_t size);
		void read_callback_msg(os::error ec, size_t size);
		void read_callback_msg_write(const std::vector<char>& write_buffer);
		void write_callback(os::error ec, size_t size);		

		protected:
#ifdef WIN32
		std::shared_ptr<os::windows::named_pipe> m_socket;
#elif __APPLE__
		std::shared_ptr<os::apple::named_pipe> m_socket;
#endif
		std::shared_ptr<os::async_op> m_wop, m_rop;
		std::vector<char> m_wbuf, m_rbuf;
		std::queue<std::vector<char>> m_write_queue;

		private:
		server* m_parent = nullptr;
		int64_t m_clientId;
	};
}
