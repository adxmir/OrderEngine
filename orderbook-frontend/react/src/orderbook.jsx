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
        const order = {
        action: 'placeOrder',
        type : side,
        price: Number(priceInput),
        volume: Number(volumeInput)
        };
        sendMessage(JSON.stringify(order));
        setPriceInput('');
        setVolumeInput(''); 
    };
    
    function handleSubmit(e){
        e.preventDefault();
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
        </div>
    );
}