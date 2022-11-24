import argparse
from typing import List

class EndPoint:
    def __init__(self, URI: str, numaNode: int, coreAffinityInit: int):
        self.coreAffinityInit = coreAffinityInit
        self.numaNode = numaNode
        self.URI = URI

    def __str__(self):
        representation = ""
        for key, val in self.__dict__.items():
            if key != "URI":
                if isinstance(val, List):
                    strList = ""
                    for v in val:
                        strList += str(v) + " "
                    if strList != "":
                        representation = (
                                representation
                                + self.URI
                                + "."
                                + key
                                + "="
                                + strList[:-1]
                                + "\n"
                        )
                else:
                    representation = (
                            representation + self.URI + "." + key + "=" + str(val) + "\n"
                    )
        return representation


class Driver(EndPoint):
    def __init__(
            self,
            URI: str,
            numaNode: int,
            coreAffinityInit: int,
            arrivalRate: int,
            servers: str,
            id: int,
            physicalPortNumber: int,
    ):
        super().__init__(URI, numaNode, coreAffinityInit)
        self.id = id
        self.arrivalRate = arrivalRate
        self.servers = servers
        self.physicalPortNumber = physicalPortNumber


class Server(EndPoint):
    def __init__(
            self,
            URI: str,
            numaNode: int,
            coreAffinityInit: int,
            numWorkers: int,
            id: int,
            physicalPortNumber: int,
    ):
        super().__init__(URI, numaNode, coreAffinityInit)
        self.id = id
        self.drivers = []
        self.neighbours = []
        self.numWorkers = numWorkers
        self.numNeighbours = 0
        self.receiverPhysicalPortNumber = physicalPortNumber
        self.heartbeatMgrPhysicalPortNumber = physicalPortNumber

    def addNeighbour(self, neighbour: str):
        self.neighbours.append(neighbour)
        self.numNeighbours += 1

    def addNeighbours(self, neighbours: List[str]):
        for neighbour in neighbours:
            if neighbour != self.URI:
                self.addNeighbour(neighbour)

    def addDriver(self, driver: str):
        self.drivers.append(driver)

    def addDrivers(self, drivers: List[str]):
        self.drivers = self.drivers + drivers


class GlobalInformation:
    def __init__(
            self,
            perWorkerTaskQueueSize: int,
            updatePeriod: int,
            avgRewardStepSize: float,
            weightDecay: float,
            batchSize: int,
            expBeta: float,
            betaOne: float,
            betaTwo: float,
            LRScheduler: str,
            minEta: float,
            maxEta: float,
            warmStartPeriod: int,
            normalizationFactor: float,
            executionTimeDistribution: str,
            meanExecutionTime: List[str],
            maxOnFlyMsgs: int,
            numTaskTypes: int,
            JBTThreshold: int,
            criticLR: float,
            mActorLR: float,
            sActorLR: float,
            heartBeatPeriod: int,
            migrationCost: float,
            stealingInitiationCost: float,
            failingCost: float,
            failedStealingCost: float,
            meanRewardMovingAvgNumSamples: int,
            startRecording: int,
            numLatencySamples: int,
            numHistorySamples: int,
            consensusPeriod: int,
            taskQueueSizeAvgNumSamples: int,
            actorOrder: str,
            criticOrder: str,
            criticTDLambda: float,
            actorsTDLambda: float,
            optimization: str,
            stealing: str,
    ):
        self.perWorkerTaskQueueSize = perWorkerTaskQueueSize
        self.updatePeriod = updatePeriod
        self.avgRewardStepSize = avgRewardStepSize
        self.weightDecay = weightDecay
        self.batchSize = batchSize
        self.expBeta = expBeta
        self.betaOne = betaOne
        self.betaTwo = betaTwo
        self.LRScheduler = LRScheduler
        self.minEta = minEta
        self.maxEta = maxEta
        self.warmStartPeriod = warmStartPeriod
        self.normalizationFactor = normalizationFactor
        self.executionTimeDistribution = executionTimeDistribution
        self.meanExecutionTime = meanExecutionTime
        self.maxOnFlyMsgs = maxOnFlyMsgs
        self.numTaskTypes = numTaskTypes
        self.JBTThreshold = JBTThreshold

        self.criticLR = criticLR
        self.mActorLR = mActorLR
        self.sActorLR = sActorLR

        self.heartBeatPeriod = heartBeatPeriod
        self.migrationCost = migrationCost
        self.stealingInitiationCost = stealingInitiationCost
        self.failingCost = failingCost
        self.failedStealingCost = failedStealingCost
        self.meanRewardMovingAvgNumSamples = meanRewardMovingAvgNumSamples
        self.startRecording = startRecording
        self.numLatencySamples = numLatencySamples
        self.numHistorySamples = numHistorySamples
        self.consensusPeriod = consensusPeriod
        self.taskQueueSizeAvgNumSamples = taskQueueSizeAvgNumSamples

        self.actorOrder = actorOrder
        self.criticOrder = criticOrder

        self.criticTDLambda = criticTDLambda
        self.actorsTDLambda = actorsTDLambda

        self.optimization = optimization
        self.stealing = stealing

    def __str__(self):
        representation = ""
        for key, val in self.__dict__.items():
            if isinstance(val, List):
                strList = ""
                print("!!!!!")
                for v in val:
                    strList += str(v) + " "
                if strList != "":
                    representation = representation + key + "=" + strList[:-1] + "\n"
            elif isinstance(val, float):
                representation = (
                        representation + key + "=" + "{:.10f}".format(val) + "\n"
                )
            else:
                representation = representation + key + "=" + str(val) + "\n"
        return representation


def generateConfiguration(
        hostStrs: List[str],
        workerChar: List[int],
        distribution: str,
        utilization: float,
        app: str,
        normalizationFactor: float,
        JBTThreshold: int,
        criticLR: float,
        mActorLR: float,
        sActorLR: float,
        heartBeatPeriod: int,
        migrationCost: float,
        stealingInitiationCost: float,
        failingCost: float,
        failedStealingCost: float,
        perWorkerTaskQueueSize: int,
        startRecording: int,
        numLatencySamples: int,
        numHistorySamples: int,
        avgRewardStepSize: float,
        weightDecay: float,
        batchSize: int,
        expBeta: float,
        LRScheduler: str,
        maxOnFlyMsgs: int,
        consensusPeriod: int,
        taskQueueSizeAvgNumSamples: int,
        actorOrder: str,
        criticOrder: str,
        criticTDLambda: float,
        actorsTDLambda: float,
        optimization: str,
        stealing: str,
) -> None:

    Hosts = {}
    Ports = {}
    serversPerHost = {}
    driversPerHost = {}
    physicalPorts = {}
    LastUsedCore = {}
    numaCoreMap = {}
    serversCount = {}
    driversCount = {}
    for hostStr in hostStrs:
        (
            hostName,
            hostIP,
            hostPort,
            numNICPorts,
            numNUMAs,
            firstCoreIndex,
            numCores,
            numServers,
            numDrivers,
        ) = hostStr.split(":")
        Hosts[hostName] = hostIP
        Ports[hostName] = int(hostPort)
        serversPerHost[hostName] = []
        driversPerHost[hostName] = []
        physicalPorts[hostName] = int(numNICPorts)
        LastUsedCore[hostName] = int(firstCoreIndex)
        numaCoreMap[hostName] = [
            i * (int(numCores) / int(numNUMAs)) for i in range(int(numNUMAs) + 1)
        ]
        serversCount[hostName] = int(numServers)
        driversCount[hostName] = int(numDrivers)

    id = 0
    totalNumWorkers = 0

    def findIndex(arr, val):
        for i, v in enumerate(arr):
            if v > val:
                return i - 1
        raise ValueError

    lastI = 0
    servers = []
    serverURIS = []
    for host in serversCount:
        num = serversCount[host]
        for i in range(lastI, num + lastI):
            coreAffinityInit = LastUsedCore[host]
            numaNode = findIndex(numaCoreMap[host], coreAffinityInit)
            URI = Hosts[host] + ":" + str(Ports[host])
            numWorkers = int(workerChar[i % len(workerChar)])
            totalNumWorkers += numWorkers
            if (numWorkers + coreAffinityInit + 2) > numaCoreMap[host][numaNode + 1]:
                numaNode += 1
                coreAffinityInit = numaCoreMap[host][numaNode]
            assert numaNode < len(numaCoreMap[host])
            server = Server(
                URI=URI,
                coreAffinityInit=coreAffinityInit,
                numWorkers=numWorkers,
                numaNode=numaNode,
                id=id,
                physicalPortNumber=i % physicalPorts[host],
            )
            # plus 2 -> heartbeat and gateway threads!
            LastUsedCore[host] = coreAffinityInit + numWorkers + 2
            assert LastUsedCore[host] <= numaCoreMap[host][-1]
            Ports[host] += 1
            id += 1
            servers.append(server)
            serversPerHost[host].append(server)
            serverURIS.append(URI)
        lastI += num
    drivers = []
    driversURIs = []
    for host in driversCount:
        num = driversCount[host]
        for i in range(num):
            coreAffinityInit = LastUsedCore[host]
            numaNode = findIndex(numaCoreMap[host], coreAffinityInit)
            URI = Hosts[host] + ":" + str(Ports[host])
            assert numaNode < len(numaCoreMap[host])
            if coreAffinityInit > numaCoreMap[host][numaNode + 1]:
                numaNode += 1
                coreAffinityInit = numaCoreMap[host][numaNode]
            assert numaNode < len(numaCoreMap[host])
            driver = Driver(
                URI=URI,
                numaNode=numaNode,
                coreAffinityInit=coreAffinityInit,
                servers=serverURIS,
                arrivalRate=0,
                id=id,
                physicalPortNumber=i % physicalPorts[host],
            )
            driversURIs.append(URI)
            drivers.append(driver)
            LastUsedCore[host] += 1
            Ports[host] += 1
            id += 1
            driversPerHost[host].append(driver)
            assert LastUsedCore[host] <= numaCoreMap[host][-1]

    distribution, characteristic = distribution.split(";")
    if distribution == "Exp":
        mean = int(characteristic)
        numTaskTypes = 1
    else:
        numTaskTypes = 0
        characteristic = characteristic.split(",")
        totalShares = 0
        sumMeans = 0
        for char in characteristic:
            share, mean = char.split(":")
            share = float(share)
            mean = int(mean)
            sumMeans += mean * share
            totalShares += share
            numTaskTypes += 1
        print(sumMeans)
        print(totalShares)
        mean = sumMeans / totalShares
    arrivalRate = totalNumWorkers * utilization * (10**9 / mean) / len(drivers)

    print("len drivers {}".format(len(drivers)))
    print("total num workers {}".format(totalNumWorkers))
    print("mean {}".format(mean))
    print("arrival rate {}".format(arrivalRate))
    globalInformation = GlobalInformation(
        perWorkerTaskQueueSize=perWorkerTaskQueueSize,
        updatePeriod=64,
        avgRewardStepSize=avgRewardStepSize,
        weightDecay=weightDecay,
        batchSize=batchSize,
        expBeta=expBeta,
        betaOne=0.9,
        betaTwo=0.999,
        LRScheduler=LRScheduler,
        minEta=0.001,
        maxEta=1,
        warmStartPeriod=100,
        normalizationFactor=normalizationFactor,
        executionTimeDistribution=distribution,
        meanExecutionTime=characteristic,
        maxOnFlyMsgs=maxOnFlyMsgs,
        numTaskTypes=numTaskTypes,
        JBTThreshold=JBTThreshold,
        criticLR=criticLR,
        mActorLR=mActorLR,
        sActorLR=sActorLR,
        heartBeatPeriod=heartBeatPeriod,
        migrationCost=migrationCost,
        stealingInitiationCost=stealingInitiationCost,
        failingCost=failingCost,
        failedStealingCost=failedStealingCost,
        meanRewardMovingAvgNumSamples=20,
        startRecording=startRecording,
        numLatencySamples=numLatencySamples,
        numHistorySamples=numHistorySamples,
        consensusPeriod=consensusPeriod,
        taskQueueSizeAvgNumSamples=taskQueueSizeAvgNumSamples,
        actorOrder=actorOrder,
        criticOrder=criticOrder,
        criticTDLambda=criticTDLambda,
        actorsTDLambda=actorsTDLambda,
        optimization=optimization,
        stealing=stealing,
    )
    # print(globalInformation)

    with open("configuration.config", "w") as file:
        file.write("######### Nodes ########\n")
        file.write("\n\n\n")
        for host in Hosts:
            if serversPerHost[host]:
                file.write(host + ".servers=")
                file.write(" ".join([server.URI for server in serversPerHost[host]]))
                file.write("\n")
        file.write("\n\n\n")
        file.write("######### Servers ########\n")
        file.write("\n\n\n")
        for server in servers:
            server.addDrivers(driversURIs)
            server.addNeighbours(serverURIS)
            file.write(server.__str__())
            file.write("\n\n\n")
        file.write("######### Drivers ########\n")
        file.write("\n\n\n")
        for driver in drivers:
            driver.arrivalRate = arrivalRate
            file.write(driver.__str__())
            file.write("\n\n\n")
        file.write("######### Global ########\n")
        file.write("\n\n\n")
        file.write(globalInformation.__str__())

    scripts = {}
    for host in Hosts:
        scripts[host] = "sudo bash cluster.sh -cf configuration.config -a {}".format(
            app
        )
        if serversPerHost[host]:
            scripts[host] = (
                    scripts[host]
                    + " --shost "
                    + Hosts[host]
                    + " --sport "
                    + str(Ports[host] - len(serversPerHost[host]) - len(driversPerHost[host]))
                    + " --sid "
                    + str(serversPerHost[host][0].id)
                    + " --snum "
                    + str(len(serversPerHost[host]))
            )
        if driversPerHost[host]:
            scripts[host] = (
                    scripts[host]
                    + " --dhost "
                    + Hosts[host]
                    + " --dport "
                    + str(Ports[host] - len(driversPerHost[host]))
                    + " --did "
                    + str(driversPerHost[host][0].id)
                    + " --dnum "
                    + str(len(driversPerHost[host]))
            )
        with open(host.upper() + ".sh", "w") as file:
            file.write(scripts[host])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Create configuration and running script."
    )
    parser.add_argument(
        "--hosts",
        type=str,
        nargs="+",
        help="hostName:ipAddress:port:numNICPorts:numNUMAs:numCores:numServers:numDrivers, i.e. hps1:192.168.0.5:31850:2:2:128:4:0 hps2:192.168.0.3:31850:2:8:128:8:0",
    )
    parser.add_argument(
        "--workerChar",
        type=int,
        nargs="+",
        help="number of workers on each server, i.e. for a three server case: 1 4 2",
    )
    parser.add_argument(
        "--dist",
        type=str,
        help="Exp, MultiModal, Multimodal, i.e. Exp;50000, Exp;20000, MultiModal:{9:50000,1:500000}",
    )
    parser.add_argument(
        "--app", type=str, default="SyntheticData", help="SyntheticData"
    )
    parser.add_argument("--util", type=float, default=0.8)
    parser.add_argument("--normalizationFactor", type=float, default=0.00001)
    parser.add_argument("--JBTThreshold", type=int, default=30)
    parser.add_argument("--mActorLR", type=float, default=0.0001)
    parser.add_argument("--sActorLR", type=float, default=0.0001)
    parser.add_argument("--criticLR", type=float, default=0.0001)
    parser.add_argument("--heartBeatPeriod", type=int, default=100)
    parser.add_argument("--migrationCost", type=float, default=0.0001)
    parser.add_argument("--stealingInitiationCost", type=float, default=0.0001)
    parser.add_argument("--failingCost", type=float, default=100)
    parser.add_argument("--failedStealingCost", type=float, default=0.1)
    parser.add_argument("--perWorkerTaskQueueSize", type=int, default=500)
    parser.add_argument("--startRecording", type=int, default=1000000)
    parser.add_argument("--numLatencySamples", type=int, default=1000000)
    parser.add_argument("--numHistorySamples", type=int, default=3000000)
    parser.add_argument("--avgRewardStepSize", type=float, default=0.0002)
    parser.add_argument("--weightDecay", type=float, default=0.0001)
    parser.add_argument("--batchSize", type=int, default=1)
    parser.add_argument("--expBeta", type=float, default=1)
    parser.add_argument("--LRScheduler", type=str, default="constant")
    parser.add_argument("--maxOnFlyMsgs", type=int, default=1024)
    parser.add_argument("--consensusPeriod", type=int, default=16)
    parser.add_argument("--taskQueueSizeAvgNumSamples", type=int, default=20)
    parser.add_argument("--actorOrder", type=str, default="second")
    parser.add_argument("--criticOrder", type=str, default="second")
    parser.add_argument("--criticTDLambda", type=float, default=0)
    parser.add_argument("--actorsTDLambda", type=float, default=0)
    parser.add_argument("--optimization", type=str, default="adamW")
    parser.add_argument("--stealing", type=str, default="on")

    args = parser.parse_args()
    generateConfiguration(
        hostStrs=args.hosts,
        workerChar=args.workerChar,
        distribution=args.dist,
        utilization=args.util,
        app=args.app,
        normalizationFactor=args.normalizationFactor,
        JBTThreshold=args.JBTThreshold,
        mActorLR=args.mActorLR,
        sActorLR=args.sActorLR,
        criticLR=args.criticLR,
        heartBeatPeriod=args.heartBeatPeriod,
        migrationCost=args.migrationCost,
        stealingInitiationCost=args.stealingInitiationCost,
        failingCost=args.failingCost,
        failedStealingCost=args.failedStealingCost,
        perWorkerTaskQueueSize=args.perWorkerTaskQueueSize,
        startRecording=args.startRecording,
        numLatencySamples=args.numLatencySamples,
        numHistorySamples=args.numHistorySamples,
        avgRewardStepSize=args.avgRewardStepSize,
        weightDecay=args.weightDecay,
        batchSize=args.batchSize,
        expBeta=args.expBeta,
        LRScheduler=args.LRScheduler,
        maxOnFlyMsgs=args.maxOnFlyMsgs,
        consensusPeriod=args.consensusPeriod,
        taskQueueSizeAvgNumSamples=args.taskQueueSizeAvgNumSamples,
        actorOrder=args.actorOrder,
        criticOrder=args.criticOrder,
        criticTDLambda=args.criticTDLambda,
        actorsTDLambda=args.actorsTDLambda,
        optimization=args.optimization,
        stealing=args.stealing,
    )
