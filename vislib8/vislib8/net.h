// This is the type for a net callback.  
// id is a unique identifier for this connection, it is gaurenteed to not be the same
// as any other currently open connection, but may be reused after a connection is closed.
// This function must return a vmallocd handle to data to be sent, or 0 to signal a close.
// if dt is NULL, this connection just started (sz == 1) or the other
// end closed it (sz == 0).  If dt != NULL, then dt is a pointer to sz bytes of recieved data.
// tmstmp is the number of seconds since program start.
typedef u32 vncallback( const u8* dt, u32 sz, u32 id, f32 tmstmp );

void vnlisten( u32 port, vncallback cb );
void vntick( void );

// How long in seconds to leave a coneection open.
#define vn_timeout ( 3.0f )