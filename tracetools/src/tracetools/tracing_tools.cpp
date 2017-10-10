#include "tracetools/tracing_tools.h"
#ifdef WITH_LTTNG
#include "tp_call.h"
#endif
#include <execinfo.h>
#include <sstream>

namespace ros {
	void TracingTools::trace_task_init(const char* name, const char* owner) {
#ifdef WITH_LTTNG
		std::ostringstream oss;
		oss << name << " of " << owner;
		tracepoint(roscpp, task_start, oss.str().c_str());
#endif
	}
	void TracingTools::trace_node_init(const char* node_name, unsigned int roscpp_version) {
#ifdef WITH_LTTNG
        tracepoint(roscpp, init_node, node_name, roscpp_version);
#endif
	}
	void TracingTools::trace_call_start(const void* ptr_ref, const void* data,
			const uint64_t trace_id) {
#ifdef WITH_LTTNG
		tracepoint(roscpp, callback_start, ptr_ref, data, trace_id);
#endif
	}
	void TracingTools::trace_call_end(const void* ptr_ref, const void* data,
			const uint64_t trace_id) {
#ifdef WITH_LTTNG
		tracepoint(roscpp, callback_end, ptr_ref, data, trace_id);
#endif
	}

	void TracingTools::trace_message_processed(const char* message_name,
			const void* callback_ref, const uint32_t receipt_time_sec,
			const uint32_t receipt_time_nsec) {
#ifdef WITH_LTTNG
			tracepoint(roscpp, message_processed, message_name, callback_ref,
					receipt_time_sec, receipt_time_nsec);
#endif
	}


	std::string get_backtrace(int index) {
#ifdef WITH_LTTNG
		const int bufsize = 50;
		void* bt_buffer[bufsize];
		int size = backtrace(bt_buffer, bufsize);
		char** symbols = backtrace_symbols(bt_buffer, size);

		std::ostringstream oss;
		if(index < size) {
			if(index < 0) {
				// add full backtrace (excepting ourselves and our immediate
				// caller)
				for(int i = 2; i < size; ++i) {
					oss << symbols[i] << "|";
				}
			} else {
				oss << symbols[index];
			}
		} else
			oss << "Invalid index " << index << " requested, only have "
				<< size << " backtrace entries";

		free(symbols);

		return oss.str();
#else
		return "";
#endif
	}

	std::string get_symbol(void* funptr)
	{
#ifdef WITH_LTTNG
		char** symbols = backtrace_symbols(&funptr, 1);
		std::string result(symbols[0]);
		free(symbols);
		return result;
#else
		return "";
#endif
	}

	void TracingTools::trace_name_info(const void* const_ptr, const void* ptr) {
#ifdef WITH_LTTNG
		void* func_ptr = const_cast<void*>(const_ptr);
		char** symbols = backtrace_symbols(&func_ptr, 1);
		// check if we just got a pointer
		if(symbols[0][0] == '[') {
			tracepoint(roscpp, ptr_name_info, get_backtrace().c_str(), ptr);
		} else {
			tracepoint(roscpp, ptr_name_info, symbols[0], ptr);
		}
		free(symbols);
#endif
	}
	/*void TracingTools::trace_cb_call(const void* ptr_ref, const void* data, const
					uint64_t trace_id) {
		tracepoint(roscpp, ptr_call, ptr_ref, data, trace_id);
	}*/

	const void* TracingTools::getCallbackFunction(const CallbackInterfacePtr& cb) {
#ifdef WITH_LTTNG
		return cb.get();
#else
		return NULL;
#endif
	}

	std::string TracingTools::getCallbackInfo(const CallbackInterfacePtr& cb) {
#ifdef WITH_LTTNG
		void* funptr = const_cast<void*>(getCallbackFunction(cb));
		char** symbols = backtrace_symbols(&funptr, 1);
		std::string result(symbols[0]);
		free(symbols);
		return typeid(*cb).name()  + std::string(" ") + result;
#else
		return "";
#endif
	}

	void TracingTools::trace_timer_scheduled(const void* callback_ref,
					const void* timer_ref) {
#ifdef WITH_LTTNG
		tracepoint(roscpp, timer_scheduled, callback_ref, timer_ref);
#endif
	}

	void trace_callback_wrapper(void* func_ptr,
		const SubscriptionCallbackHelperPtr& helper) {
#ifdef WITH_LTTNG
		tracepoint(roscpp, ptr_name_info,
			get_symbol(func_ptr).c_str(),
			helper.get());
#endif
	}

	void
	trace_link(const char* element_name,
			const void* caller_ref,
			const void* in_data_ref,
			const void* out_data_ref,
			const uint64_t trace_id) {
#ifdef WITH_LTTNG
#ifdef NO_LINK_BACKTRACES
		tracepoint(roscpp, trace_link, element_name, user_name,
						in_data_ref, out_data_ref, trace_id, NULL);
#else
		tracepoint(roscpp, trace_link, element_name, typeid(caller_ref).name(),
				caller_ref, in_data_ref, out_data_ref, trace_id,
				get_backtrace().c_str());
#endif
#endif
	}

    void TracingTools::trace_new_connection(const char* local_hostport_arg,
											  const char* remote_hostport_arg,
											  const void* channel_ref_arg,
											  const char* channel_type_arg,
											  const char* name_arg,
											  const char* data_type_arg) {
#ifdef WITH_LTTNG
			tracepoint(roscpp, new_connection, local_hostport_arg, remote_hostport_arg, channel_ref_arg,
					   channel_type_arg, name_arg, data_type_arg);
#endif
		}
		void TracingTools::trace_publisher_link_handle_message(const void* channel_ref_arg,
														const void* buffer_ref_arg
		) {
#ifdef WITH_LTTNG
			tracepoint(roscpp, publisher_link_handle_message, channel_ref_arg, buffer_ref_arg);
#endif
		}

	void TracingTools::trace_publisher_message_queued(const char* topic_arg,
										   const void* buffer_ref_arg) {
#ifdef WITH_LTTNG
        tracepoint(roscpp, publisher_message_queued, topic_arg, buffer_ref_arg);
#endif
	}
	void TracingTools::trace_subscriber_link_message_write(const void* message_ref_arg,
												const void* channel_ref_arg) {
#ifdef WITH_LTTNG
		tracepoint(roscpp, subscriber_link_message_write, message_ref_arg, channel_ref_arg);
#endif
	}
	void TracingTools::trace_subscriber_link_message_dropped(const void* message_ref_arg) {
#ifdef WITH_LTTNG
		tracepoint(roscpp, subscriber_link_message_dropped, message_ref_arg);
#endif
	}

	void TracingTools::trace_subscription_message_queued(const char* topic_arg,
											  const void* buffer_ref_arg,
											  const void* queue_ref_arg,
											  const void* callback_ref_arg,
											  const void* message_ref_arg,
											  int receipt_time_sec_arg,
											  int receipt_time_nsec_arg) {
#ifdef WITH_LTTNG
        tracepoint(roscpp, subscription_message_queued, topic_arg, buffer_ref_arg, queue_ref_arg,
		callback_ref_arg, message_ref_arg, receipt_time_sec_arg, receipt_time_nsec_arg);
#endif
	}
	void TracingTools::trace_subscriber_callback_added(const void* queue_ref_arg,
											const void* callback_ref_arg,
											const char* type_info_arg,
											const char* data_type_arg,
											const char* source_name_arg,
											int queue_size_arg) {
#ifdef WITH_LTTNG
    tracepoint(roscpp, subscriber_callback_added, queue_ref_arg, callback_ref_arg, type_info_arg, data_type_arg,
	source_name_arg, queue_size_arg);
#endif
}

	void TracingTools::trace_queue_delay(const char* queue_name,
					const void* ptr_ref, const void* data,
					const uint32_t entry_time_sec, const uint32_t entry_time_nsec)
	{
#ifdef WITH_LTTNG
		tracepoint(roscpp, queue_delay, queue_name, ptr_ref, data,
				entry_time_sec, entry_time_nsec);
#endif
	}

}