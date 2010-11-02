#ifndef	__unp_rtt_h
#define	__unp_rtt_h

#include	"unp.h"

struct rtt_info {
  int		rtt_rtt;	/* most recent measured RTT, in m-seconds */
  int		rtt_srtt;	/* smoothed RTT estimator, in m-seconds */
  int		rtt_rttvar;	/* smoothed mean deviation, in m-seconds */
  int		rtt_rto;	/* current RTO to use, in m-seconds */
  int		rtt_nrexmt;	/* # times retransmitted: 0, 1, 2, ... */
  uint32_t	rtt_base;	/* # m-sec since 1/1/1970 at start */
};

#define	RTT_RXTMIN      1000	/* min retransmit timeout value, in seconds */
#define	RTT_RXTMAX      3000	/* max retransmit timeout value, in seconds */
#define	RTT_MAXNREXMT 	12	/* max # times to retransmit */

				/* function prototypes */
void	 my_rtt_debug(struct rtt_info *);
void	 my_rtt_init(struct rtt_info *);
void	 my_rtt_newpack(struct rtt_info *);
int      my_rtt_start(struct rtt_info *);
void	 my_rtt_stop(struct rtt_info *, uint32_t);
int	 my_rtt_timeout(struct rtt_info *);
uint32_t my_rtt_ts(struct rtt_info *);

extern int	rtt_d_flag;	/* can be set to nonzero for addl info */

#endif	/* __unp_rtt_h */
