
#include "MaterialViewer/material_viewer_server.hpp"

struct MaterialViewerServerTest
{
    struct Input
    {
        SocketContext* socket_ctx;
        int32 port;
    } input;

    struct Sync
    {
        volatile int8 allocated;
    } sync;

    struct Exec
    {
        MaterialViewerServerTest* thiz;
        inline int8 operator()() const
        {
            thiz->server = MaterialViewerServer::allocate(*thiz->input.socket_ctx, thiz->input.port);
            thiz->sync.allocated = 1;
            thiz->server.mloop(*thiz->input.socket_ctx);

            return 0;
        };
    } exec;

    thread_t thread;
    MaterialViewerServer server;

    inline void start(SocketContext* p_ctx, const int32 p_port)
    {
        this->sync.allocated = 0;
        this->input = Input{p_ctx, p_port};
        this->exec.thiz = this;
        this->thread = Thread::spawn_thread(this->exec);
    };

    inline void free(SocketContext* p_ctx)
    {
        this->server.free(*p_ctx);
        Thread::wait_for_end_and_terminate(this->thread, -1);
    };

    inline void sync_wait_for_allocated()
    {
        while (!this->sync.allocated)
        {
        }
    };
};

struct MaterialViwerClientTest {

    struct Input
    {
        SocketContext* socket_ctx;
    } input;

    struct Sync
    {
        volatile int8 allocated;
    } sync;

    struct Exec {
        MaterialViwerClientTest* thiz;
        inline int8 operator()()const{
            thiz->client = MaterialViewerClient::allocate(*thiz->input.socket_ctx, 8000);
            thiz->sync.allocated = 1;
            thiz->client.listen_for_responses(*thiz->input.socket_ctx);

            return 0;
        };
    } exec;

    MaterialViewerClient client;
    thread_t thread;


    inline void start(SocketContext* p_ctx)
    {
        this->sync.allocated = 0;
        this->input = Input{p_ctx};
        this->exec.thiz = this;
        this->thread = Thread::spawn_thread(this->exec);
    };

    inline void free(SocketContext* p_ctx)
    {
        this->client.free(*p_ctx);
        Thread::wait_for_end_and_terminate(this->thread, -1);
    };

    inline void sync_wait_for_allocated()
    {
        while (!this->sync.allocated)
        {
        }
    };
};


int main()
{
    SocketContext l_socket_context = SocketContext::allocate();

    MaterialViewerServerTest l_server;
    l_server.start(&l_socket_context, 8000);
    l_server.sync_wait_for_allocated();

    MaterialViwerClientTest l_client;
    l_client.start(&l_socket_context);
    l_client.sync_wait_for_allocated();

    l_client.client.ENGINE_THREAD_START(slice_int8_build_rawstr("E:/GameProjects/GameEngineLinux/_asset/asset/MaterialViewer_Package/asset.db"));
    MaterialViewerClient::GetAllMaterialsReturn l_materials = l_client.client.GET_ALL_MATERIALS();

    l_materials.free();


    l_client.free(&l_socket_context);
    l_server.free(&l_socket_context);

    l_socket_context.free();
    memleak_ckeck();

    return 0;
};