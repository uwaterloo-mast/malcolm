import paramiko
from typing import List, Dict, Optional
from HostConfiguration import HostConfiguration

ACCOUNT = ""
DIRECTORY = ""
SSH_PASSWORD = ""
SUDO_PASSWORD = ""

class Cluster:
    def __init__(self, hostsConfigurations: List[HostConfiguration]):
        self.sshClients: Dict[str, paramiko.SSHClient] = {}
        for conf in hostsConfigurations:
            self.sshClients[conf.hostName] = paramiko.SSHClient()
            self.sshClients[conf.hostName].set_missing_host_key_policy(
                paramiko.AutoAddPolicy()
            )
            self.sshClients[conf.hostName].connect(conf.sshName, 22, ACCOUNT, SSH_PASSWORD)

    def runCommand(
        self,
        command: str,
        servers: Optional[List[str]] = None,
        enterPass: bool = False,
        printOutPut: bool = True,
        printCommand: bool = True,
        returnOutput: bool = False,
    ):
        if servers == None:
            servers = [hostName for hostName in self.sshClients]

        for server in servers:
            assert server in self.sshClients

        if printCommand:
            print(command)

        for server in servers:
            session = self.sshClients[server].get_transport().open_session()
            paramiko.agent.AgentRequestHandler(session)
            session.set_combine_stderr(True)
            session.get_pty()
            session.exec_command(command)
            stdin = session.makefile("wb", -1)
            stdout = session.makefile("rb", -1)
            # (stdin, stdout, _) = self.sshClients[server].exec_command(command)

            if enterPass:
                stdin.write(SUDO_PASSWORD + "\n")
                stdin.flush()

            result = []
            if printOutPut:
                for line in stdout.read().splitlines():
                    result.append(line.decode("utf-8"))
                    print("host_%s - %s" % (server, line.decode("utf-8")))

            if returnOutput:
                return result

    def __del__(self):
        for client in self.sshClients.values():
            print("closing the connection!")
            client.close()
