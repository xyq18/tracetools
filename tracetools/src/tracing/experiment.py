import subprocess
import random
from .ctf_converter import convert, ListTarget
import os

ROSCPP_META_EVENTS = [
    "roscpp:new_connection",
    "roscpp:callback_added",
    "roscpp:timer_added",
    "roscpp:task_start",
    "roscpp:init_node",
    "nodelet:task_start",
    "nodelet:init",
    "tf2:task_start",
    "roscpp:ptr_name_info",
]
ROSCPP_TRACE_EVENTS = [
    "roscpp:callback_start",
    "roscpp:callback_end",
    "roscpp:publisher_link_handle_message",
    "roscpp:subscription_message_queued",
    "roscpp:subscriber_callback_start",
    "roscpp:subscriber_callback_end",
    "roscpp:publisher_message_queued",
    "roscpp:subscriber_link_message_write",
    "roscpp:subscriber_link_message_drop",
    "roscpp:subscriber_callback_added",
    "roscpp:subscriber_callback_wrapper",
    "roscpp:message_processed",
    "roscpp:trace_link",
    "roscpp:ptr_call",
    "roscpp:timer_scheduled",
    "roscpp:queue_delay"
]

STD_CONTEXT = [
    "vpid",
    "procname",
    "vtid",
    "perf:thread:instructions",
    "perf:thread:cycles",
    "perf:thread:cpu-cycles"
]


class TraceExperiment(object):
    def __init__(self, exp_args, userspace_events, context=STD_CONTEXT, meta_events=ROSCPP_META_EVENTS, working_directory=None):
        """

        :param exp_args:
        :param userspace_events:
        :param context:
        :param meta_events:
        :param working_directory: The directory to execute in. If None, the directory of the exp_args[0] will be used
        :return:
        """
        self._exp_args = exp_args
        self._events = userspace_events + meta_events
        self._context = context
        self._session_name = None
        self._session_data_dir = None
        self._working_directory = working_directory if working_directory is not None else os.path.dirname(exp_args[0])

    def create(self, session_name=None, **kwargs):
        """
        Set up a new lttng session.
        :param session_name: Session name for lttng. Random if not provided
        :param kwargs: Any keyword arguments will be passed on to the subprocess call.
        :return: The session name
        """
        cmd = ["lttng", "create"]
        if session_name is not None:
            cmd.append(session_name)

        create_proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True, **kwargs)
        stdoutdata, _ = create_proc.communicate()
        # at least in current versions of lttng, the last piece of output from create is the session data directory
        self._session_data_dir = stdoutdata.split()[-1]

        self._session_name = session_name
        for event in self._events:
            subprocess.check_call(["lttng", "enable-event", "-u", event], stdout=subprocess.PIPE)
        for ctx_event in self._context:
            subprocess.check_call(["lttng", "add-context", "-u", "-t", ctx_event], stdout=subprocess.PIPE)

        return self._session_name

    def collect_data(self, names=None):
        """
        Collects data from the session. Implicitly stops it.
        :param names: The event names to return, or None for everything (default)
        :return: A list with the collected events
        """
        l = ListTarget()
        convert(self._session_data_dir, l, names)
        return l.contents

    def run(self, repeats=1):
        try:
            subprocess.check_call(["lttng", "start"])
            for i in range(0, repeats):
                subprocess.check_call(self._exp_args, cwd=self._working_directory)
            subprocess.check_call(["lttng", "stop"])
        except subprocess.CalledProcessError as e:
            print(e)