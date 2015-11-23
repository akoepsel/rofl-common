#ifndef ROFL_EXAMPLES_TCPSERVER_H
#define ROFL_EXAMPLES_TCPSERVER_H 1

#include <inttypes.h>
#include <signal.h>
#include <map>

#include <rofl/common/crofbase.h>

namespace rofl {
namespace examples {



/**
 * @ingroup common_howto_tcpserver
 *
 * @brief	A simple server with OpenFlow endpoint connecting to a control plane
 *
 * A simple example for using rofl to connect to a controller entity.
 */
class tcpserver :
		public rofl::crofbase
{
public:

	/**
	 * @brief	tcpserver destructor
	 */
	virtual
	~tcpserver()
	{

	};

	/**
	 * @brief	tcpserver constructor
	 */
	tcpserver() :
		keep_on_running(true),
		dptid(0)
	{

	};

	/** @cond EXAMPLES */

public:

	/**
	 *
	 */
	int
	run(
			int argc, char** argv)
	{
		start();

		while (keep_on_running) {
			struct timespec ts;
			ts.tv_sec  = 2;
			ts.tv_nsec = 0;
			pselect(0, NULL, NULL, NULL, &ts, NULL);
		}

		std::cerr << ">>> journal log <<<" << std::endl;
		std::cerr << crofbase::get_journal() << std::endl;

		return 0;
	};

	/**
	 * @brief	Static main routine for class cetherswitch
	 *
	 * Runs main event loop. Does not return.
	 *
	 * @param argc number of arguments given to main function
	 * @param argv array of pointers to arguments given to main function
	 */
	void
	start()
	{
		rofl::openflow::cofhello_elem_versionbitmap vbitmap;
		vbitmap.add_ofp_version(rofl::openflow12::OFP_VERSION);
		vbitmap.add_ofp_version(rofl::openflow13::OFP_VERSION);

		crofbase::set_versionbitmap(vbitmap);
		crofbase::dpt_sock_listen(rofl::csockaddr(AF_INET, "0.0.0.0", 6653));
	};

	/**
	 *
	 */
	void
	stop()
	{
		keep_on_running = false;
	};

private:

	/**
	 * @brief	Called after establishing the associated OpenFlow control channel.
	 *
	 * This method is called once the associated OpenFlow control channel has
	 * been established, i.e., its main connection has been accepted by the remote site.
	 *
	 * @param dpt datapath instance
	 */
	virtual void
	handle_dpt_open(
			rofl::crofdpt& dpt)
	{
		std::cerr << "datapath attached dptid=" << dpt.get_dptid() << std::endl;

		dpt.set_conn(rofl::cauxid(0)).set_trace(true);

		dpt.send_port_desc_stats_request(rofl::cauxid(0), 0, 5);
	};

	/**
	 * @brief	Called after termination of associated OpenFlow control channel.
	 *
	 * This method is called once the associated OpenFlow control channel has
	 * been terminated, i.e., its main connection has been closed from the
	 * remote site. The rofl::crofdpt instance itself is not destroyed, unless
	 * its 'remove_on_channel_close' flag has been set to true during its
	 * construction.
	 *
	 * @param dpt datapath instance
	 */
	virtual void
	handle_dpt_close(
			const rofl::cdptid& dptid)
	{
		std::cerr << "datapath detached dptid=" << dptid << std::endl;

		stop();

		for (auto dptid : crofbase::dpt_keys()) {
			std::cerr << ">>> dpt journal for dptid: " << dptid.str() << " <<<" << std::endl;
			std::cerr << crofbase::get_dpt(dptid).get_journal() << std::endl;
			for (auto auxid : crofbase::get_dpt(dptid).keys()) {
				std::cerr << ">>> conn journal for dptid: " << dptid.str() << ", auxid: " << auxid.str() << " <<<" << std::endl;
				std::cerr << crofbase::get_dpt(dptid).get_conn(auxid).get_journal() << std::endl;
				std::cerr << ">>> tcp journal for dptid: " << dptid.str() << ", auxid: " << auxid.str() << " <<<" << std::endl;
				std::cerr << crofbase::get_dpt(dptid).get_conn(auxid).get_tcp_journal() << std::endl;
			}
		}
	};

	/**
	 * @brief	OpenFlow Features-Request message received.
	 *
	 * @param ctl   reference to crofctl instance
	 * @param auxid OpenFlow auxiliary connection identifier
	 * @param msg   Features-Request message received
	 */
	virtual void
	handle_port_desc_stats_reply(
			rofl::crofdpt& dpt,
			const rofl::cauxid& auxid,
			rofl::openflow::cofmsg_port_desc_stats_reply& msg)
	{
		std::cerr << "port description received; " << dpt.get_ports() << std::endl;
	};

	/** @endcond */

public:

	friend std::ostream&
	operator<< (std::ostream& os, const tcpserver& server) {

		return os;
	};

private:

	bool                        keep_on_running;

	rofl::cdptid                dptid;
};

}; // namespace examples
}; // namespace rofl

#endif /* ROFL_EXAMPLES_TCPSERVER_H */
