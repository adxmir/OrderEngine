import {useState, useEffect} from 'react'
import useWebSocket from 'react-use-websocket'
import LevelTwoQuotes from './quote.jsx'

export function ShowBook(){
    const WS_URL = 'ws://127.0.0.1:8090';
    const { sendMessage, lastMessage, readyState } = useWebSocket(WS_URL)

    const [orderbook , setOrderBook] = useState([]);
    const [volumeInput, setVolumeInput] = useState('');
    const [priceInput, setPriceInput] = useState('');
    const [orderCounter, setOrderCount] = useState(0);
    const[orderResult, setResult] = useState("");
    
    useEffect(() => { 
    console.log('truing');
    if(lastMessage != null){
        try {
            const data = JSON.parse(lastMessage.data);
            if(data.type == 'OrderBook Snapshot'){
                setOrderBook(data);
            }
            else if(data.type == 'Order Counter'){
                setOrderCount(data.orders);
            }
        } catch(err) {
            console.error(err);
        }
    }
    }, [lastMessage])

    const placeOrder = (side) => {
        if (readyState !== WebSocket.OPEN) return;
        if(priceInput.trim() == "" || volumeInput.trim() == ""){
            setResult("Price or volume not declared");
            clearResultMessage();
            return;
        }
        const price_ = Number(priceInput);
        const volume_ = Number(volumeInput);
        const order = {
        action: 'placeOrder',
        type : side,
        price: price_,
        volume: volume_
        };
        sendMessage(JSON.stringify(order));
        setPriceInput('');
        setVolumeInput(''); 
        setResult("Order succesfully carried out at price {" + price_ + "} volume {" + volume_ + "}");
        clearResultMessage();
    };
    
    function handleSubmit(e){
        e.preventDefault();
    }

    function clearResultMessage(){
        setTimeout( ()  => setResult(""), 3000);
    }

    return(
        <div>
            <LevelTwoQuotes bids = {orderbook.bids} asks = {orderbook.asks}></LevelTwoQuotes>
            <h1>
                Place orders
            </h1>
            <form onSubmit = {(e) => handleSubmit (e,'buy')}>
                <input
                type = "tel"
                placeholder = "Price"
                value = {priceInput}
                onChange={(e) => setPriceInput(e.target.value)}>
                </input>
                <input 
                type = "tel"
                placeholder = "Volume"
                value = {volumeInput}
                onChange = {(e) => setVolumeInput(e.target.value)}>
                </input>
                <button type="buyButton" onClick={() => placeOrder('BUY')}>Buy</button>
                <button type="sellButton" onClick={() => placeOrder('SELL')}>Sell</button>
            </form>
            <label> {orderCounter} </label>
            <br/>
            <label> {orderResult} </label>
        </div>
    );
}