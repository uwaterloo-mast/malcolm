trap "exit" INT TERM ERR
trap "kill 0" EXIT

stringContain() { [ -z "$1" ] || { [ -z "${2##*$1*}" ] && [ -n "$2" ]; }; }

CURRDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

shost=""
dhost=""
sport=""
dport=""
sid=""
did=""
snum=""
dnum=""
drivers=""
config_file=""
other_configurations=""
app=SyntheticData

while [ "$1" != "" ]; do
  case "$1" in
  "--shost")
    shift
    shost="$1"
    ;;
  "--dhost")
    shift
    dhost="$1"
    ;;
  "--sport")
    shift
    sport="$1"
    ;;
  "--dport")
    shift
    dport="$1"
    ;;
  "--sid")
    shift
    sid="$1"
    ;;
  "--did")
    shift
    did="$1"
    ;;
  "--snum")
    shift
    snum="$1"
    ;;
  "--dnum")
    shift
    dnum="$1"
    ;;
  "--config_file" | "-cf")
    shift
    config_file="$1"
    ;;
  "--app" | "-a")
    shift
    app="$1"
    ;;
  "--config" | "-ocf")
    shift
    other_configurations="${other_configurations} --config $1"
    ;;
  "--help" | "-h")
    echo "Please Note the --num_servers and --num_drivers should be passed"
    exit 1
    ;;
  *)
    echo -n "Unknown Command : $1"
    ;;
  esac
  shift
done

echo $other_configurations
echo $config_file

declare -A numaNodes

while IFS= read -r line; do
  for driver in $drivers; do
    if stringContain "$driver.numaNode" $line; then
      x=($(echo $line | tr "=" "\n"))
      n=${x[1]}
      numaNodes["$driver"]=$n
      echo $line
      echo $n
    fi
  done
done <"$config_file"

for h in $shost; do
  sudo ./../build/${app}Servers --config_file $config_file $other_configurations --config host=$shost --config port=$sport --config id=$sid --config num=$snum &
done

sleep 1

for h in $dhost; do
  sudo ./../build/${app}Drivers --config_file $config_file $other_configurations --config host=$dhost --config port=$dport --config id=$did --config num=$dnum &
done

wait
