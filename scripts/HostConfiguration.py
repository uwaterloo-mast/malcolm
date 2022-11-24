from typing import List


class HostConfiguration:
    def __init__(
        self,
        sshName: str,
        hostName: str,
        hostIP: str,
        hostPort: int,
        numNICPorts: int,
        numNUMAs: int,
        firstCoreIndex: int,
        numCores: int,
        numServers: int,
        numDrivers: int,
        numWorkerList: List[int],
    ) -> None:
        self.sshName = sshName
        self.hostName = hostName
        self.hostIP = hostIP
        self.hostPort = hostPort
        self.numNICPorts = numNICPorts
        self.numNUMAs = numNUMAs
        self.firstCoreIndex = firstCoreIndex
        self.numCores = numCores
        self.numServers = numServers
        self.numDrivers = numDrivers

        self.numWorkerList = numWorkerList
        assert len(self.numWorkerList) == self.numServers

    def __str__(self) -> str:
        return ":".join(
            [
                self.hostName,
                self.hostIP,
                str(self.hostPort),
                str(self.numNICPorts),
                str(self.numNUMAs),
                str(self.firstCoreIndex),
                str(self.numCores),
                str(self.numServers),
                str(self.numDrivers),
            ]
        )

    def printNumWokerList(self) -> str:
        return " ".join([str(n) for n in self.numWorkerList])


def printHostsStr(hostsConfigurations: List[HostConfiguration]) -> str:
    return " ".join([str(c) for c in hostsConfigurations])


def printWorkerCount(hostsConfigurations: List[HostConfiguration]) -> str:
    s = ""
    for hostConfig in hostsConfigurations:
        if hostConfig.numServers > 0:
            s += hostConfig.printNumWokerList() + " "
    return s
