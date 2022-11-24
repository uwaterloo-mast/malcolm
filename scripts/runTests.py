import os
import time
import argparse
from typing import List
from HostConfiguration import HostConfiguration, printHostsStr, printWorkerCount
from runCluster import Cluster, ACCOUNT, DIRECTORY

policyNames = ["DLB", "POWD", "JSQ", "JBT", "RAND"]

buildCommand = (
    "cd {}/{}/ && mkdir build ; cd build && "
    "cmake .. -DCMAKE_BUILD_TYPE=Release "
    "-DDEFINE_{} -DDEFINE_{} "
    "-DDEFINE_{} -DDEFINE_{} "
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=1 && "
    "cp compile_commands.json ../ && "
    "make -j"
)

buildCommands = {
    "DLB": buildCommand.format(DIRECTORY, "DLB", "DLB=ON", "POWD=OFF", "JSQ=OFF", "JBT=OFF"),
    "POWD": buildCommand.format(DIRECTORY, "POWD", "POWD=ON", "DLB=OFF", "JSQ=OFF", "JBT=OFF"),
    "JSQ": buildCommand.format(DIRECTORY, "JSQ", "JSQ=ON", "POWD=OFF", "DLB=OFF", "JBT=OFF"),
    "JBT": buildCommand.format(DIRECTORY, "JBT", "JBT=ON", "POWD=OFF", "JSQ=OFF", "DLB=OFF"),
    "RAND": buildCommand.format(DIRECTORY, "RAND", "JBT=OFF", "POWD=OFF", "JSQ=OFF", "DLB=OFF"),
}

utilizationConfigs = {
    "homogeneous": {
        "exp": {
            "DLB": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "JBT":  [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "JSQ":  [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "POWD": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "RAND": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
        },
        "bimodal": {
            "DLB": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "JBT": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "JSQ": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "POWD": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "RAND": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
        },
        "hyperExp-1": {
            "DLB": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "JBT": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "JSQ": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "POWD": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "RAND": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
        },
        "hyperExp-2": {
            "DLB": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "JBT": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "JSQ": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "POWD": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
            "RAND": [0.8, 0.9, 0.94, 0.98, 0.99, 1],
        },
    },
    "heterogeneous": {
        "exp": {
            "DLB": [0.4, 0.7, 0.85, 0.94, 0.98, 0.99, 1],
            "POWD": [0.4, 0.65, 0.75, 0.8, 0.85],
            "JSQ": [0.4, 0.5, 0.6, 0.65, 0.7],
            "JBT": [0.4, 0.5, 0.6, 0.65, 0.7],
            "RAND": [0.4, 0.5, 0.55, 0.6, 0.65],
        },
        "bimodal": {
            "DLB": [0.4, 0.7, 0.85, 0.94, 0.98, 0.99, 1],
            "POWD": [0.4, 0.65, 0.75, 0.8, 0.85],
            "JSQ": [0.4, 0.5, 0.6, 0.65, 0.7],
            "JBT": [0.4, 0.5, 0.6, 0.65, 0.7],
            "RAND": [0.4, 0.5, 0.55, 0.6, 0.65],
        },
        "hyperExp-1": {
            "DLB": [0.4, 0.7, 0.85, 0.94, 0.98, 0.99, 1],
            "POWD": [0.4, 0.65, 0.75, 0.8, 0.85],
            "JSQ": [0.4, 0.5, 0.6, 0.65, 0.7],
            "JBT": [0.4, 0.5, 0.6, 0.65, 0.7],
            "RAND": [0.4, 0.5, 0.55, 0.6, 0.65],
        },
        "hyperExp-2": {
            "DLB": [0.4, 0.7, 0.85, 0.94, 0.98, 0.99, 1],
            "POWD": [0.4, 0.65, 0.75, 0.8, 0.85],
            "JSQ": [0.4, 0.5, 0.6, 0.65, 0.7],
            "JBT": [0.4, 0.5, 0.6, 0.65, 0.7],
            "RAND": [0.4, 0.5, 0.55, 0.6, 0.65],
        },
    }
}

additionalParams = {
    "DLB": "",
    "POWD": "",
    "JSQ": "",
    "JBT": "--JBTThreshold {}".format(30),
    "RAND": "",
}

distributionNames = [
    "bimodal",
    "exp",
    "hyperExp-1",
    "hyperExp-2",
]

distributionParams = {
    "exp": '"Exp;75000"',
    "bimodal": '"MultiModal;8:50000,2:250000"',
    "hyperExp-1": '"HYPEXP;10:50000,10:500000"',
    "hyperExp-2": '"HYPEXP;15:50000,4:500000,1:5000000"',
}

timeouts = {
    "exp": 70,
    "bimodal": 70,
    "hyperExp-1": 120,
    "hyperExp-2": 120,
}

normalizationFactors = {
    "exp": 0.001,
    "bimodal": 0.001,
    "hyperExp-1": 0.001,
    "hyperExp-2": 0.001,
}

perWorkerTaskQueueSizes = {
    "exp": 32,
    "bimodal": 32,
    "hyperExp-1": 128,
}

appMap = {
    "bimodal": "SyntheticData",
    "exp": "SyntheticData",
    "hyperExp-1": "SyntheticData",
    "hyperExp-2": "SyntheticData",
}

# Host configurations
SAMPLE1_HOM = HostConfiguration(
    "sample1.com",
    "sample1",
    "192.168.0.1",
    31850,
    2,
    2,
    0,
    128,
    5,
    4,
    [10, 10, 10, 10, 10],
)

SAMPLE2_HOM = HostConfiguration(
    "sample2.com",
    "sample2",
    "192.168.0.2",
    31850,
    2,
    2,
    0,
    128,
    5,
    4,
    [10, 10, 10, 10, 10],
)

SAMPLE1_HET = HostConfiguration(
    "sample1.com",
    "sample1",
    "192.168.0.1",
    31850,
    2,
    2,
    0,
    128,
    4,
    8,
    [12, 12, 12, 12],
)

SAMPLE2_HET = HostConfiguration(
    "sample2.com",
    "sample2",
    "192.168.0.2",
    31850,
    2,
    2,
    0,
    128,
    10,
    4,
    [4, 4, 4, 4, 4, 4, 4, 4, 4, 4],
)


hostsConfigHomogeneous = [SAMPLE1_HOM, SAMPLE2_HOM]
hostsConfigHeterogeneous = [SAMPLE1_HET, SAMPLE2_HET]
hostConfigs = {
    "homogeneous": hostsConfigHomogeneous,
    "heterogeneous": hostsConfigHeterogeneous,
}

def runTest(
    distributions: List[str],
    settings: List[str],
    policies: List[str],
    criticLR: float,
    mActorLR: float,
    sActorLR: float,
    compile: bool,
    hearBeatPeriod: int,
    migrationCost: float,
    stealingInitiationCost: float,
    failingCost: float,
    failedStealingCost: float,
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
):
    global policyNames
    global buildCommands
    global distributionNames
    global distributionParams
    global appMap
    global hostsConfigHomogeneous
    global hostsConfigHeterogeneous
    global timeouts
    global normalizationFactors
    global perWorkerTaskQueueSizes

    for dist in distributions:
        assert dist in distributionNames
    for p in policies:
        assert p in policyNames

    for setting in settings:
        hostsConfigurations = hostConfigs[setting]

        clusterShell = Cluster(hostsConfigurations)
        for policy in policies:

            if compile == 1:
                clusterShell.runCommand(
                    buildCommands[policy],
                )

            for dist in distributions:
                utilizations = utilizationConfigs[setting][dist][policy]

                for util in utilizations:
                    print(
                        "===================== Policy = {},"
                        " Dist = {}, Util = {} =====================".format(
                            policy, dist, util
                        )
                    )

                    assert policy in additionalParams
                    assert dist in distributionParams
                    assert dist in appMap

                    try:
                        # Clean up previous histories and logs
                        clusterShell.runCommand(
                            "cd {}/{}/scripts/ && rm -f *.bin ; rm *.txt".format(DIRECTORY, policy),
                        )

                        # Create config files on hosts
                        clusterShell.runCommand(
                            "cd {}/{}/scripts/ && python3 generateConfig.py --hosts {} "
                            "--workerChar {} --dist {} --util {} --normalizationFactor {} --app {} {} "
                            "--criticLR {} --mActorLR {} --sActorLR {} --heartBeatPeriod {} --migrationCost {} "
                            "--stealingInitiationCost {} --failingCost {} --failedStealingCost {} "
                            "--perWorkerTaskQueueSize {} --startRecording {} --numLatencySamples {} "
                            "--numHistorySamples {} --avgRewardStepSize {} --weightDecay {} --batchSize {} "
                            "--expBeta {} --LRScheduler {} --maxOnFlyMsgs {} --consensusPeriod {} "
                            "--taskQueueSizeAvgNumSamples {} --actorOrder {} --criticOrder {} --criticTDLambda {} "
                            "--actorsTDLambda {} --optimization {} --stealing {} ".format(
                                DIRECTORY,
                                policy,
                                printHostsStr(hostsConfigurations),
                                printWorkerCount(hostsConfigurations),
                                distributionParams[dist],
                                util,
                                normalizationFactors[dist],
                                appMap[dist],
                                additionalParams[policy],
                                criticLR,
                                mActorLR,
                                sActorLR,
                                hearBeatPeriod,
                                migrationCost,
                                stealingInitiationCost,
                                failingCost,
                                failedStealingCost,
                                perWorkerTaskQueueSizes[dist],
                                startRecording,
                                numLatencySamples,
                                numHistorySamples,
                                avgRewardStepSize,
                                weightDecay,
                                batchSize,
                                expBeta,
                                LRScheduler,
                                maxOnFlyMsgs,
                                consensusPeriod,
                                taskQueueSizeAvgNumSamples,
                                actorOrder,
                                criticOrder,
                                criticTDLambda,
                                actorsTDLambda,
                                optimization,
                                stealing,
                            )
                        )

                        # Run servers and drivers on hosts for timeout seconds
                        for conf in hostsConfigurations:
                            clusterShell.runCommand(
                                "cd {}/{}/scripts/ && sudo bash {} |& tee {}.txt".format(
                                    DIRECTORY, policy, conf.hostName.upper() + ".sh", conf.hostName
                                ),
                                [conf.hostName],
                                enterPass=True,
                                printOutPut=False
                            )
                        time.sleep(timeouts[dist])

                        # Kill servers and drivers
                        clusterShell.runCommand(
                            "sudo pkill -9 -f SyntheticData",
                            enterPass=True,
                            printOutPut=False,
                            printCommand=False
                        )

                        # Retrieve logs and histories
                        for host in hostsConfigurations:
                            os.system(
                                "scp -r {}@{}:{}/{}/scripts/*.txt .".format(
                                    ACCOUNT, host.sshName, DIRECTORY, policy
                                )
                            )
                        os.system(
                            "rename 's/.txt/\_{}\_{}\_{}\_{}.txt/' *.txt".format(
                                setting, policy, dist, util, policy,
                            )
                        )
                        os.system("mv *.txt ./{}/".format(policy))
                    except BaseException as err:
                        print(f"*********** Error ({err=}) occurred! ***********")
        del clusterShell


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Create configuration and running script,"
        " the script assumes that all DLB repos are in DIRECTORY,"
        " and the account owner is ACCOUNT. ACCOUNT and DIRECTORY"
        " are configurable in runCluster.py"
    )
    parser.add_argument(
        "--setting",
        type=str,
        default=["homogeneous", "heterogeneous"],
        nargs="+"
    )
    parser.add_argument(
        "--policy",
        type=str,
        default=["DLB", "POWD", "JSQ", "JBT", "RAND"],
        nargs="+",
        help="Scheduling policy: DLB, POWD, JSQ, JBT, RAND",
    )
    parser.add_argument(
        "--dist",
        type=str,
        default=["exp", "bimodal", "hyperExp-1", "hyperExp-2"],
        nargs="+",
        help="Workload distribution: bimodal, exp, hyperExp-1, hyperExp-2",
    )
    parser.add_argument("--criticLR", type=float, default=0.002)
    parser.add_argument("--mActorLR", type=float, default=0.001)
    parser.add_argument("--sActorLR", type=float, default=0.001)
    parser.add_argument("--hearBeatPeriod", type=int, default=100)
    parser.add_argument("--batchSize", type=int, default=20)
    parser.add_argument("--expBeta", type=float, default=1)
    parser.add_argument("--criticTDLambda", type=float, default=0.6)
    parser.add_argument("--actorsTDLambda", type=float, default=0.6)
    parser.add_argument("--avgRewardStepSize", type=float, default=0.001)
    parser.add_argument("--weightDecay", type=float, default=0.0001)
    parser.add_argument("--taskQueueSizeAvgNumSamples", type=int, default=20)
    parser.add_argument("--migrationCost", type=float, default=0.0001)
    parser.add_argument("--stealingInitiationCost", type=float, default=0.0001)
    parser.add_argument("--failingCost", type=float, default=10)
    parser.add_argument("--failedStealingCost", type=float, default=0)
    parser.add_argument("--consensusPeriod", type=int, default=100)
    parser.add_argument("--compile", type=int, default=0)
    parser.add_argument("--startRecording", type=int, default=0)
    parser.add_argument("--numLatencySamples", type=int, default=50000)
    parser.add_argument("--numHistorySamples", type=int, default=3000000)
    parser.add_argument("--maxOnFlyMsgs", type=int, default=1024)
    parser.add_argument("--LRScheduler", type=str, default="constant")
    parser.add_argument("--actorOrder", type=str, default="sort")
    parser.add_argument("--criticOrder", type=str, default="customized")
    parser.add_argument("--optimization", type=str, default="adamW")
    parser.add_argument("--stealing", type=str, default="on")

    args = parser.parse_args()

    runTest(
        distributions=args.dist,
        settings=args.setting,
        policies=args.policy,
        criticLR=args.criticLR,
        mActorLR=args.mActorLR,
        sActorLR=args.sActorLR,
        compile=args.compile,
        hearBeatPeriod=args.hearBeatPeriod,
        migrationCost=args.migrationCost,
        stealingInitiationCost=args.stealingInitiationCost,
        failingCost=args.failingCost,
        failedStealingCost=args.failedStealingCost,
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
