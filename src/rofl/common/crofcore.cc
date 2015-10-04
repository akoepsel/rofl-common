/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * crofcore.cc
 *
 *  Created on: 11.04.2015
 *  Revised on: 04.10.2015
 *      Author: andreas
 */

#include "crofcore.h"

using namespace rofl;

/*static*/std::set<crofcore*>   crofcore::rofcores;
/*static*/crwlock               crofcore::rofcores_rwlock;


crofcore::~crofcore()
{
	/* close listening sockets */
	close_dpt_socks();
	close_ctl_socks();

	/* close all crofdpt instances */
	drop_dpts();

	/* close all crofctl instances */
	drop_ctls();

	AcquireReadWriteLock rwlock(rofcores_rwlock);
	crofcore::rofcores.erase(this);
	if (crofcore::rofcores.empty()) {
		crofcore::initialize();
	}
}



crofcore::crofcore() :
	thread(this),
	transactions(this),
	generation_is_defined(false),
	cached_generation_id((uint64_t)((int64_t)-1))
{
	AcquireReadWriteLock rwlock(rofcores_rwlock);
	if (crofcore::rofcores.empty()) {
		crofcore::initialize();
	}
	crofcore::rofcores.insert(this);
}



/*static*/
void
crofcore::initialize()
{}



/*static*/
void
crofcore::terminate()
{}



void
crofcore::role_request_rcvd(
		crofctl& ctl,
		uint32_t role,
		uint64_t rcvd_generation_id)
{
	if (generation_is_defined &&
			(rofl::openflow::cofrole::distance((int64_t)rcvd_generation_id, (int64_t)cached_generation_id) < 0)) {
		if ((rofl::openflow13::OFPCR_ROLE_MASTER == role) || (rofl::openflow13::OFPCR_ROLE_SLAVE == role)) {
			throw eRoleRequestStale();
		}
	} else {
		cached_generation_id = rcvd_generation_id;
		generation_is_defined = true;
	}

	// in either case: send current generation_id value back to controller
	ctl.set_role().set_generation_id(cached_generation_id);

	switch (role) {
	case rofl::openflow13::OFPCR_ROLE_MASTER: {

		// iterate over all attached controllers and check for an existing master
		for (std::map<cctlid, crofctl*>::iterator
				it = rofctls.begin(); it != rofctls.end(); ++it) {

			// ignore ctl who called this method
			if (it->second->get_ctlid() == ctl.get_ctlid())
				continue;

			// find any other controller and set them back to role SLAVE
			if (rofl::openflow13::OFPCR_ROLE_MASTER == it->second->get_role().get_role()) {
				it->second->set_role().set_role(rofl::openflow13::OFPCR_ROLE_SLAVE);
			}
		}

		// set new master async-config to template retrieved from of-config (or default one)
		ctl.set_async_config() = ctl.get_async_config_role_default_template();

		ctl.set_role().set_role(rofl::openflow13::OFPCR_ROLE_MASTER);

	} break;
	case rofl::openflow13::OFPCR_ROLE_SLAVE: {

		ctl.set_async_config() = ctl.get_async_config_role_default_template();
		ctl.set_role().set_role(rofl::openflow13::OFPCR_ROLE_SLAVE);

	} break;
	case rofl::openflow13::OFPCR_ROLE_EQUAL: {

		ctl.set_async_config() = ctl.get_async_config_role_default_template();
		ctl.set_role().set_role(rofl::openflow13::OFPCR_ROLE_EQUAL);

	} break;
	case rofl::openflow13::OFPCR_ROLE_NOCHANGE:
	default: {
		// let crofctl_impl send a role-reply with the controller's unaltered current role
	}
	}
}




void
crofcore::send_packet_in_message(
		const cauxid& auxid,
		uint32_t buffer_id,
		uint16_t total_len,
		uint8_t reason,
		uint8_t table_id,
		uint64_t cookie,
		uint16_t in_port, // for OF1.0
		rofl::openflow::cofmatch &match,
		uint8_t *data,
		size_t datalen)
{
	bool sent_out = false;

	for (std::map<cctlid, crofctl*>::iterator
			it = rofctls.begin(); it != rofctls.end(); ++it) {

		crofctl& ctl = *(it->second);

		if (not ctl.is_established()) {
			continue;
		}

		ctl.send_packet_in_message(
				auxid,
				buffer_id,
				total_len,
				reason,
				table_id,
				cookie,
				in_port, // for OF1.0
				match,
				data,
				datalen);

		sent_out = true;
	}

	if (not sent_out) {
		throw eRofCoreNotConnected("rofl::crofcore::send_packet_in_message() not connected");
	}
}



void
crofcore::send_flow_removed_message(
		const cauxid& auxid,
		rofl::openflow::cofmatch& match,
		uint64_t cookie,
		uint16_t priority,
		uint8_t reason,
		uint8_t table_id,
		uint32_t duration_sec,
		uint32_t duration_nsec,
		uint16_t idle_timeout,
		uint16_t hard_timeout,
		uint64_t packet_count,
		uint64_t byte_count)
{
	bool sent_out = false;

	for (std::map<cctlid, crofctl*>::iterator
			it = rofctls.begin(); it != rofctls.end(); ++it) {

		crofctl& ctl = *(it->second);

		if (not ctl.is_established()) {
			continue;
		}

		ctl.send_flow_removed_message(
				auxid,
				match,
				cookie,
				priority,
				reason,
				table_id,
				duration_sec,
				duration_nsec,
				idle_timeout,
				hard_timeout,
				packet_count,
				byte_count);

		sent_out = true;
	}

	if (not sent_out) {
		throw eRofCoreNotConnected("rofl::crofcore::send_flow_removed_message() not connected");
	}
}



void
crofcore::send_port_status_message(
		const cauxid& auxid,
		uint8_t reason,
		const rofl::openflow::cofport& port)
{
	bool sent_out = false;

	for (std::map<cctlid, crofctl*>::iterator
			it = rofctls.begin(); it != rofctls.end(); ++it) {

		crofctl& ctl = *(it->second);

		if (not ctl.is_established()) {
			continue;
		}

		ctl.send_port_status_message(
				auxid,
				reason,
				port);

		sent_out = true;
	}

	if (not sent_out) {
		throw eRofCoreNotConnected("rofl::crofcore::send_port_status_message() not connected");
	}
}




int
crofcore::listen(
		const csockaddr& baddr)
{
	int sd;
	int rc;
	int type = SOCK_STREAM;
	int protocol = IPPROTO_TCP;
	int backlog = 10;

	/* open socket */
	if ((sd = ::socket(baddr.get_family(), type, protocol)) < 0) {
		throw eSysCall("socket()");
	}

	/* make socket non-blocking */
	long flags;
	if ((flags = ::fcntl(sd, F_GETFL)) < 0) {
		throw eSysCall("fnctl() F_GETFL");
	}
	flags |= O_NONBLOCK;
	if ((rc = ::fcntl(sd, F_SETFL, flags)) < 0) {
		throw eSysCall("fcntl() F_SETGL");
	}

	if ((SOCK_STREAM == type) && (IPPROTO_TCP == protocol)) {
		int optval = 1;

		// set SO_REUSEADDR option on TCP sockets
		if ((rc = ::setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (int*)&optval, sizeof(optval))) < 0) {
			throw eSysCall("setsockopt() SOL_SOCKET, SO_REUSEADDR");
		}

#if 0
		int on = 1;
		if ((rc = ::setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on))) < 0) {
			throw eSysCall("setsockopt(SOL_SOCKET, SO_REUSEPORT)");
		}
#endif

		// set TCP_NODELAY option on TCP sockets
		if ((rc = ::setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (int*)&optval, sizeof(optval))) < 0) {
			throw eSysCall("setsockopt() IPPROTO_TCP, TCP_NODELAY");
		}

		// set SO_RCVLOWAT
		if ((rc = ::setsockopt(sd, SOL_SOCKET, SO_RCVLOWAT, (int*)&optval, sizeof(optval))) < 0) {
			throw eSysCall("setsockopt() SOL_SOCKET, SO_RCVLOWAT");
		}

		// read TCP_NODELAY option for debugging purposes
		socklen_t optlen = sizeof(int);
		int optvalc;
		if ((rc = ::getsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (int*)&optvalc, &optlen)) < 0) {
			throw eSysCall("getsockopt() IPPROTO_TCP, TCP_NODELAY");
		}
	}

	/* bind to local address */
	if ((rc = ::bind(sd, baddr.ca_saddr, (socklen_t)(baddr.salen))) < 0) {
		throw eSysCall("bind");
	}

	/* listen on socket */
	if ((rc = ::listen(sd, backlog)) < 0) {
		throw eSysCall("listen");
	}

	return sd;
}



void
crofcore::handle_wakeup(
		cthread& thread)
{

}



void
crofcore::handle_timeout(
		cthread& thread, uint32_t timer_id, const std::list<unsigned int>& ttypes)
{

}



void
crofcore::handle_read_event(
		cthread& thread, int fd)
{
	std::map<csockaddr, int>::iterator it;

	{
		/* incoming datapath connection */
		AcquireReadLock rlock(dpt_sockets_rwlock);
		if ((it = find_if(dpt_sockets.begin(), dpt_sockets.end(),
				csocket_find_by_sock_descriptor(fd))) != dpt_sockets.end()) {
			(new crofconn(this))->tcp_accept(fd, versionbitmap, crofconn::MODE_DATAPATH);
		}
	}

	{
		/* incoming controller connection */
		AcquireReadLock rlock(ctl_sockets_rwlock);
		if ((it = find_if(ctl_sockets.begin(), ctl_sockets.end(),
				csocket_find_by_sock_descriptor(fd))) != ctl_sockets.end()) {
			(new crofconn(this))->tcp_accept(fd, versionbitmap, crofconn::MODE_CONTROLLER);
		}
	}
}



void
crofcore::handle_established(
		crofconn& conn, uint8_t ofp_version)
{
	/* openflow connection has been established */

	switch (conn.get_mode()) {
	case crofconn::MODE_CONTROLLER: {
		/* if datapath for dpid already exists add connection there
		 * or create new crofdpt instance */
		if (not has_dpt(cdpid(conn.get_dpid()))) {
			add_dpt().add_conn(&conn);
		} else {
			set_dpt(cdpid(conn.get_dpid())).add_conn(&conn);
		}
	} break;
	case crofconn::MODE_DATAPATH: {
		/* add a new controller instance and add connection there
		 * we cannot identify a replaced controller connection */
		add_ctl().add_conn(&conn);
	} break;
	default: {
		delete &conn;
	};
	}
}



void
crofcore::handle_accept_failed(
		crofconn& conn)
{
	delete &conn;
}



void
crofcore::handle_negotiation_failed(
		crofconn& conn)
{
	delete &conn;
}



void
crofcore::handle_closed(
		crofconn& conn)
{
	delete &conn;
}




