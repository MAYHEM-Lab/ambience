#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#undef index

#include <arch/display.hpp>
#include <tos/debug/assert.hpp>
#include <tos/debug/log.hpp>
#include <tos/ft.hpp>
#include <tos/gfx/painter.hpp>
#include <tos/semaphore.hpp>

namespace tos::x86::x11 {
struct window {};
} // namespace tos::x86::x11

namespace tos::x86 {
struct display_impl {
    display_impl(const gfx::dimensions& dims);

    void flush() const {
        XFlush(m_display);
        XFlushGC(m_display, m_gc);
    }

    int convert_color(tos::gfx::rgb8 rgb) {
        XColor col;
        col.pixel = 0;
        col.flags = DoRed | DoGreen | DoBlue;
        col.red = rgb.red * 65535 / 255;
        col.green = rgb.green * 65535 / 255;
        col.blue = rgb.blue * 65535 / 255;
        auto cmap = XDefaultColormap(m_display, 0);
        auto res = XAllocColor(m_display, cmap, &col);
        Assert(0 != res);
        return col.pixel;
    }

    void create_colormap() {
        XColor tmp[255];
        for (int i = 0; i < 255; i++) {
            auto& col = tmp[i];
            col.pixel = i;
            col.flags = DoRed | DoGreen | DoBlue;
            col.red = i * 65535 / 255;
            col.blue = i * 0;
            col.green = i * 0;
        }

        auto cmap = XDefaultColormap(m_display, 0);
        for (auto& i : tmp) {
            auto res = XAllocColor(m_display, cmap, &i);
            Assert(0 != res);
        }

        XSetForeground(m_display, m_gc, tmp[128].pixel);
        flush();
    }

    void run() {
        while (true) {
            while (XEventsQueued(m_display, QueuedAfterFlush) == 0) {
                tos::this_thread::yield();
            }

            XEvent event;
            XNextEvent(m_display, &event);
            if (event.type == KeyRelease) {
                char text[4];
                auto res =
                    XLookupString(&event.xkey, text, std::size(text), nullptr, nullptr);
                LOG_TRACE("Got key", res, text[0]);
                // break;
            } else if (event.type == Expose && event.xexpose.count == 0) {
                LOG_TRACE("Expose");
                //                for (auto& cmd : m_commands) {
                //                    cmd(this);
                //                }
            }
        }
        m_exited.up();
    }

    template<class CommandT>
    void add_command(const CommandT& fn) {
        fn(this);
        flush();
    }

    ~display_impl() {
        // m_exited.down();
        XFreeGC(m_display, m_gc);
        XCloseDisplay(m_display);
    }

    gfx::dimensions m_dims;
    tos::semaphore m_exited{0};
    Display* m_display;
    Window m_window;
    int m_screen;
    GC m_gc;

private:
    std::vector<std::function<void(display_impl*)>> m_commands;
};

struct x11_painter : gfx::painter {
    explicit x11_painter(display_impl& display)
        : m_display{&display} {
    }

    void draw(const gfx::line& line, const gfx::fixed_color& col) override {
        m_display->add_command([line, col](display_impl* display) {
            XSetForeground(
                display->m_display, display->m_gc, display->convert_color(col.color));

            auto l = line;
            l.from.y = display->m_dims.height - l.from.y;
            l.to.y = display->m_dims.height - l.to.y;

            XDrawLine(display->m_display,
                      display->m_window,
                      display->m_gc,
                      l.from.x,
                      l.from.y,
                      l.to.x,
                      l.to.y);
        });
    }

    void draw(const gfx::rectangle& rect, const gfx::fixed_color& col) override {
        m_display->add_command([rect, col](display_impl* display) {
            XSetForeground(
                display->m_display, display->m_gc, display->convert_color(col.color));

            XFillRectangle(display->m_display,
                           display->m_window,
                           display->m_gc,
                           rect.corner.x,
                           display->m_dims.height - rect.corner.y - rect.size.height,
                           rect.size.width,
                           rect.size.height);
        });
    }

    display_impl* m_display;
};

display_impl::display_impl(const gfx::dimensions& dims)
    : m_dims(dims) {
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        LOG_ERROR("Cannot open display");
        return;
    }

    m_screen = DefaultScreen(m_display);
    m_window = XCreateSimpleWindow(m_display,
                                   RootWindow(m_display, m_screen),
                                   0,
                                   0,
                                   dims.width,
                                   dims.height,
                                   5,
                                   BlackPixel(m_display, m_screen),
                                   WhitePixel(m_display, m_screen));
    XSizeHints* size_hints = XAllocSizeHints();
    size_hints->width = dims.width;
    size_hints->height = dims.height;
    size_hints->min_width = dims.width;
    size_hints->min_height = dims.height;
    size_hints->max_width = dims.width;
    size_hints->max_height = dims.height;
    size_hints->base_width = dims.width;
    size_hints->base_height = dims.height;
    size_hints->flags = PMinSize | PMaxSize | PBaseSize;
    XSetStandardProperties(
        m_display, m_window, "Tos Display", "HI!", None, nullptr, 0, size_hints);
    XSetWMNormalHints(m_display, m_window, size_hints);
    XSetWMSizeHints(m_display, m_window, size_hints, PMinSize | PMaxSize);
    XMapWindow(m_display, m_window);

    XSelectInput(m_display, m_window, KeyPressMask | KeyReleaseMask | ExposureMask);
    XGCValues values;
    values.background = 1;
    values.foreground = 1;
    m_gc = XCreateGC(m_display, m_window, GCForeground, &values);

    create_colormap();
    flush();

    // tos::launch(tos::alloc_stack, [this] { run(); });
}

display::display(const gfx::dimensions& dims)
    : m_impl(std::make_unique<display_impl>(dims)) {
}

std::unique_ptr<gfx::painter> display::get_painter() {
    return std::make_unique<x11_painter>(*m_impl);
}

display::~display() = default;
} // namespace tos::x86
