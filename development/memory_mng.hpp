namespace MemoryManagement {

    //* PREWORK
    typedef std::uint32_t	 	u32;
    /* data for acquire ioctrol */
    typedef struct hermes_usb_acqu_setup
    {
        u32 id;			/* id returned with event */
        u32 board;		/* board address */
        u32 address;	/* address offset in the acquisition mem to place the data */
        u32 size;		/* expected size in bytes */
        u32 preview_a;	/* preview expected size in bytes, if >0, an event will be generated to wake the application */
        u32 preview_b;	/* preview expected size in bytes, if >0, an event will be generated to wake the application */

    } hermes_usb_acqu_setup; //! d.	Retrieve and send DAQ-related binary messages 

    struct CustomDeleter {
        void operator()(void* ptr) const {
            free(ptr);
        }
    };
    //* PREWORK end

    void memory_mng_main(char aCmd, void* aXchangeData);

}