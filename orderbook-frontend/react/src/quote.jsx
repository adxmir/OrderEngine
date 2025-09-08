import React from 'react'
import './quote.css'

export default function LevelTwoQuotes({asks = [] ,bids = []}){
    const readyBids = bids;
    const readyAsks = asks;
    const maxL = Math.max(readyBids.length, readyAsks.length);
    return(
        <div>
            <h1> OrderBook </h1>
            <div className="table-container">
            <table>
                <thead>
                    <tr>
                        <th scope = "col">Size</th>
                        <th scope = "col">Bids</th>
                        <th scope = "col">Asks</th>
                        <th scope = "col">Size</th>
                    </tr>
                </thead>
                <tbody>
                    {[...Array(maxL)].map((_,i) => {
                        const bid = readyBids[i];
                        const ask = readyAsks[i];
                        return(
                            <tr key = {i}>
                                <td>{bid ? bid.volume : ""}</td>
                                <td>{bid ? bid.price : ""}</td>
                                <td>{ask ? ask.price : ""}</td>
                                <td>{ask ? ask.volume : ""}</td>
                            </tr>
                            )
                        })}
                </tbody>
            </table>
            </div>
        </div>
    )
}
