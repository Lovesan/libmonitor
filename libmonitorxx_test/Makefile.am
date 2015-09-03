bin_PROGRAMS = libmonitorxx_test

libmonitorxx_test_SOURCES = libmonitorxx_test.cpp

libmonitorxx_test_CPPFLAGS = -I$(srcdir) -I$(top_srcdir)/libmonitor -I$(top_srcdir)/libmonitorxx

libmonitorxx_test_LDADD = $(top_builddir)/libmonitor/libmonitor.la $(top_builddir)/libmonitorxx/libmonitorxx.la -lpthread
